/*
 * @brief Defines a configuration registry
 * @file system/ConfigurationRegistry.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <map>

#include <formats/AircraftFileFormat.h>
#include <formats/AirportFileFormat.h>
#include <types/SystemConfiguration.h>

namespace topskytower {
    namespace system {
        /**
         * @brief Describes a flight registry that contains all visible flights
         * @ingroup system
         */
        class ConfigurationRegistry {
        private:
            types::SystemConfiguration                         m_systemConfig;
            std::map<std::string, formats::AirportFileFormat*> m_airportConfigurations;
            formats::AircraftFileFormat*                       m_aircraftConfiguration;

            ConfigurationRegistry();
            void cleanup();

        public:
            /**
             * @brief Destroys the configuration registry
             */
            ~ConfigurationRegistry();

            ConfigurationRegistry(const ConfigurationRegistry& other) = delete;
            ConfigurationRegistry(ConfigurationRegistry&& other) = delete;

            ConfigurationRegistry& operator=(const ConfigurationRegistry& other) = delete;
            ConfigurationRegistry& operator=(ConfigurationRegistry&& other) = delete;

            /**
             * @brief Loads the different configuration
             * @param[in] path The path where the configurations can be found
             */
            void configure(const std::string& path);
            /**
             * @brief Returns the system configuration
             * @return The constant reference to the system configuration
             */
            const types::SystemConfiguration& systemConfiguration() const;
            /**
             * @brief Returns an airport configuration
             * @param[in] icao The airport's ICAO code
             * @return The airport configuration
             */
            const types::AirportConfiguration& airportConfiguration(const std::string& icao) const;
            /**
             * @brief Returns the parsed aircrafts
             * @return A map of aircrafts
             */
            const std::map<std::string, types::Aircraft>& aircrafts() const;
            /**
             * @brief Returns the global instance of the registry
             * @return The global instance
             */
            static ConfigurationRegistry& instance();
        };
    }
}
