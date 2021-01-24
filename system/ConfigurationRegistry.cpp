/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the configuration registry
 */

#include <filesystem>
#include <fstream>
#include <string>

#include <formats/SettingsFileFormat.h>
#include <system/ConfigurationRegistry.h>

namespace fs = std::filesystem;

using namespace topskytower;
using namespace topskytower::system;

ConfigurationRegistry::ConfigurationRegistry() :
        m_systemConfig(),
        m_runtimeConfig(),
        m_airportConfigurations(),
        m_aircraftConfiguration(nullptr) { }

ConfigurationRegistry::~ConfigurationRegistry() {
    this->cleanup(UpdateType::All);
}

void ConfigurationRegistry::cleanup(UpdateType type) {
    if (UpdateType::All == type || UpdateType::Airports == type) {
        for (auto it = this->m_airportConfigurations.begin(); this->m_airportConfigurations.end() != it; ++it)
            delete it->second;
        this->m_airportConfigurations.clear();
    }

    if (UpdateType::All == type || UpdateType::Aircrafts == type) {
        if (nullptr != this->m_aircraftConfiguration)
            delete this->m_aircraftConfiguration;
        this->m_aircraftConfiguration = nullptr;
    }

    if (UpdateType::All == type || UpdateType::System == type)
        this->m_systemConfig.valid = false;
}

void ConfigurationRegistry::configure(const std::string& path, UpdateType type) {
    this->cleanup(type);

    if (UpdateType::All == type || UpdateType::System == type) {
        formats::SettingsFileFormat settings(path + "\\TopSkyTowerSettings.txt");
        settings.parse(this->m_systemConfig);

        std::ifstream stream(path + "\\TopSkyTowerHoppies.txt");
        for (std::string line; std::getline(stream, line);) {
            if (0 != line.size()) {
                this->m_systemConfig.hoppiesCode = line;
                break;
            }
        }
    }

    if (UpdateType::All == type || UpdateType::Airports == type) {
        for (auto& entry : fs::recursive_directory_iterator(path)) {
            if (true == fs::is_regular_file(entry) && 0 == entry.path().filename().string().find("TopSkyTowerAirport")) {
                /* extract the ICAO code of the airport */
                auto filename = entry.path().filename().stem().string();
                auto icao = filename.substr(filename.length() - 4, 4);

#pragma warning(disable: 4244)
                std::transform(icao.begin(), icao.end(), icao.begin(), ::toupper);
#pragma warning(default: 4244)

                /* read the configuration */
                this->m_airportConfigurations[icao] = new formats::AirportFileFormat(entry.path().string());
            }
        }
    }

    if (UpdateType::All == type || UpdateType::Aircrafts == type)
        this->m_aircraftConfiguration = new formats::AircraftFileFormat(path + "\\TopSkyTowerAircrafts.txt");

    for (const auto& notification : std::as_const(this->m_notificationCallbacks))
        notification.second(type);
}

const types::SystemConfiguration& ConfigurationRegistry::systemConfiguration() const {
    return this->m_systemConfig;
}

void ConfigurationRegistry::setRuntimeConfiguration(const types::RuntimeConfiguration& configuration) {
    this->m_runtimeConfig = configuration;
    for (const auto& notification : std::as_const(this->m_notificationCallbacks))
        notification.second(ConfigurationRegistry::UpdateType::Runtime);
}

const types::RuntimeConfiguration& ConfigurationRegistry::runtimeConfiguration() const {
    return this->m_runtimeConfig;
}

const types::AirportConfiguration& ConfigurationRegistry::airportConfiguration(const std::string& icao) const {
    static types::AirportConfiguration __fallback;

    auto it = this->m_airportConfigurations.find(icao);
    if (this->m_airportConfigurations.cend() != it)
        return it->second->configuration();
    else
        return __fallback;
}

const std::map<std::string, types::Aircraft>& ConfigurationRegistry::aircrafts() const {
    return this->m_aircraftConfiguration->aircrafts();
}

ConfigurationRegistry& ConfigurationRegistry::instance() {
    static ConfigurationRegistry __instance;
    return __instance;
}
