/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the configuration registry
 */

#include <fstream>
#include <string>

#include <formats/SettingsFileFormat.h>
#include <system/ConfigurationRegistry.h>

using namespace topskytower;
using namespace topskytower::system;

ConfigurationRegistry::ConfigurationRegistry() :
        m_systemConfig(),
        m_airportsConfiguration(nullptr),
        m_aircraftConfiguration(nullptr) { }

ConfigurationRegistry::~ConfigurationRegistry() {
    if (nullptr != this->m_airportsConfiguration)
        delete this->m_airportsConfiguration;
    if (nullptr != this->m_aircraftConfiguration)
        delete this->m_aircraftConfiguration;
}

void ConfigurationRegistry::configure(const std::string& path) {
    formats::SettingsFileFormat settings(path + "\\TopSkyTowerSettings.txt");
    settings.parse(this->m_systemConfig);

    std::ifstream stream(path + "\\TopSkyTowerHoppies.txt");
    for (std::string line; std::getline(stream, line);) {
        if (0 != line.size()) {
            this->m_systemConfig.hoppiesCode = line;
            break;
        }
    }

    if (nullptr != this->m_airportsConfiguration)
        delete this->m_airportsConfiguration;
    this->m_airportsConfiguration = new formats::AirportFileFormat(path + "\\TopSkyTowerAirports.txt");

    if (nullptr != this->m_aircraftConfiguration)
        delete this->m_aircraftConfiguration;
    this->m_aircraftConfiguration = new formats::AircraftFileFormat(path + "\\TopSkyTowerAircrafts.txt");
}

const types::SystemConfiguration& ConfigurationRegistry::systemConfiguration() const {
    return this->m_systemConfig;
}

const types::AirportConfiguration& ConfigurationRegistry::airportConfiguration(const std::string& icao) const {
    return this->m_airportsConfiguration->configuration(icao);
}

const std::map<std::string, types::Aircraft>& ConfigurationRegistry::aircrafts() const {
    return this->m_aircraftConfiguration->aircrafts();
}

ConfigurationRegistry& ConfigurationRegistry::instance() {
    static ConfigurationRegistry __instance;
    return __instance;
}
