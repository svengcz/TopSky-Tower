/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the configuration registry
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
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
        m_aircraftConfiguration("") { }

ConfigurationRegistry::~ConfigurationRegistry() {
    this->cleanup(UpdateType::All);
}

void ConfigurationRegistry::cleanup(UpdateType type) {
    if (UpdateType::All == type || UpdateType::Airports == type)
        this->m_airportConfigurations.clear();

    if (UpdateType::All == type || UpdateType::System == type)
        this->m_systemConfig.valid = false;
}

bool ConfigurationRegistry::configure(const std::string& path, UpdateType type) {
    {
        std::lock_guard guard(this->m_configurationLock);

        this->cleanup(type);
        this->reset();

        if (UpdateType::All == type || UpdateType::System == type) {
            /* parse the global configuration */
            formats::SettingsFileFormat settings(path + "\\TopSkyTowerSettings.txt");
            if (false == settings.parse(this->m_systemConfig)) {
                this->m_errorLine = settings.errorLine();
                this->m_errorMessage = "TopSkyTowerSettings.txt:\n" + settings.errorMessage();
                return false;
            }

            /* parse the Hoppies code */
            std::ifstream stream(path + "\\TopSkyTowerHoppies.txt");
            for (std::string line; std::getline(stream, line);) {
                if (0 != line.size()) {
                    this->m_systemConfig.hoppiesCode = line;
                    break;
                }
            }

            /* parse the local settings */
            if (true == std::filesystem::exists(path + "\\TopSkyTowerSettingsLocal.txt")) {
                formats::SettingsFileFormat localSettings(path + "\\TopSkyTowerSettingsLocal.txt");
                if (false == localSettings.parse(this->m_systemConfig) && 0 != localSettings.errorLine()) {
                    this->m_errorLine = localSettings.errorLine();
                    this->m_errorMessage = "TopSkyTowerSettingsLocal.txt:\n" + localSettings.errorMessage();
                    return false;
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
                    formats::AirportFileFormat airport(entry.path().string());
                    types::AirportConfiguration airportConfig;
                    if (false == airport.parse(airportConfig)) {
                        this->m_errorLine = airport.errorLine();
                        this->m_errorMessage = filename + ".txt:\n" + airport.errorMessage();
                        return false;
                    }
                    this->m_airportConfigurations[icao] = std::move(airportConfig);
                }
            }
        }

        if (UpdateType::All == type || UpdateType::Aircrafts == type) {
            this->m_aircraftConfiguration = formats::AircraftFileFormat(path + "\\TopSkyTowerAircrafts.txt");
            if (true == this->m_aircraftConfiguration.errorFound()) {
                this->m_errorLine = this->m_aircraftConfiguration.errorLine();
                this->m_errorMessage = +"TopSkyTowerAircrafts.txt:\n" + this->m_aircraftConfiguration.errorMessage();
                return false;
            }
        }

        if (UpdateType::All == type || UpdateType::Events == type) {
            this->m_eventsConfig.events.clear();

            formats::EventRoutesFileFormat events(path + "\\TopSkyTowerEventRoutes.txt");
            if (false == events.parse(this->m_eventsConfig)) {
                if (0 != events.errorLine()) {
                    this->m_errorLine = events.errorLine();
                    this->m_errorMessage = events.errorMessage();
                }
                return false;
            }
        }
    }

    for (const auto& notification : std::as_const(this->m_notificationCallbacks))
        notification.second(type);

    return true;
}

const types::SystemConfiguration& ConfigurationRegistry::systemConfiguration() {
    std::lock_guard guard(this->m_configurationLock);
    return this->m_systemConfig;
}

void ConfigurationRegistry::setRuntimeConfiguration(const types::RuntimeConfiguration& configuration) {
    this->m_configurationLock.lock();
    this->m_runtimeConfig = configuration;
    this->m_configurationLock.unlock();

    for (const auto& notification : std::as_const(this->m_notificationCallbacks))
        notification.second(ConfigurationRegistry::UpdateType::Runtime);
}

void ConfigurationRegistry::setMetarInformation(const std::string& airport, const types::WindData& data) {
    this->m_configurationLock.lock();
    this->m_runtimeConfig.windInformation[airport] = data;
    this->m_configurationLock.unlock();

    for (const auto& notification : std::as_const(this->m_notificationCallbacks))
        notification.second(ConfigurationRegistry::UpdateType::Metar);
}

const types::RuntimeConfiguration& ConfigurationRegistry::runtimeConfiguration() {
    std::lock_guard guard(this->m_configurationLock);
    return this->m_runtimeConfig;
}

const types::AirportConfiguration& ConfigurationRegistry::airportConfiguration(const std::string& icao) {
    static types::AirportConfiguration __fallback;

    std::lock_guard guard(this->m_configurationLock);

    auto it = this->m_airportConfigurations.find(icao);
    if (this->m_airportConfigurations.cend() != it)
        return it->second;
    else
        return __fallback;
}

const std::map<std::string, types::Aircraft>& ConfigurationRegistry::aircrafts() {
    std::lock_guard guard(this->m_configurationLock);
    return this->m_aircraftConfiguration.aircrafts();
}

const types::EventRoutesConfiguration& ConfigurationRegistry::eventRoutesConfiguration() {
    std::lock_guard guard(this->m_configurationLock);
    return this->m_eventsConfig;
}

void ConfigurationRegistry::activateEvent(const std::string& event, bool active) {
    if (false == this->m_eventsConfig.valid)
        return;

    {
        std::lock_guard guard(this->m_configurationLock);
        for (auto it = this->m_eventsConfig.events.begin(); this->m_eventsConfig.events.end() != it; ++it) {
            if (it->name == event) {
                it->active = active;
                break;
            }
        }
    }

    for (const auto& notification : std::as_const(this->m_notificationCallbacks))
        notification.second(ConfigurationRegistry::UpdateType::Events);
}

ConfigurationRegistry& ConfigurationRegistry::instance() {
    static ConfigurationRegistry __instance;
    return __instance;
}
