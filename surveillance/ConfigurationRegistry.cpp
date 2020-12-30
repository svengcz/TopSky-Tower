/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the configuration registry
 */

#include <fstream>
#include <string>

#include <formats/SettingsFileFormat.h>
#include <surveillance/ConfigurationRegistry.h>

using namespace topskytower;
using namespace topskytower::surveillance;

ConfigurationRegistry::ConfigurationRegistry() :
        m_systemConfig(),
        m_airportsConfigurations(nullptr) { }

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

    if (nullptr != this->m_airportsConfigurations)
        delete this->m_airportsConfigurations;

    this->m_airportsConfigurations = new formats::AirportFileFormat(path + "\\TopSkyTowerAirports.txt");
}

const types::SystemConfiguration& ConfigurationRegistry::systemConfiguration() const {
    return this->m_systemConfig;
}

const types::AirportConfiguration& ConfigurationRegistry::airportConfiguration(const std::string& icao) const {
    return this->m_airportsConfigurations->configuration(icao);
}

ConfigurationRegistry& ConfigurationRegistry::instance() {
    static ConfigurationRegistry __instance;
    return __instance;
}
