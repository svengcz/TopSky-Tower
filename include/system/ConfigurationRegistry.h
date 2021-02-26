/*
 * @brief Defines a configuration registry
 * @file system/ConfigurationRegistry.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <functional>
#include <map>

#include <formats/AircraftFileFormat.h>
#include <formats/AirportFileFormat.h>
#include <formats/EventRoutesFileFormat.h>
#include <formats/FileFormat.h>
#include <types/RuntimeConfiguration.h>
#include <types/SystemConfiguration.h>

namespace topskytower {
    namespace system {
        /**
         * @brief Describes a flight registry that contains all visible flights
         * @ingroup system
         */
        class ConfigurationRegistry : public formats::FileFormat {
        public:
            /**
             * @brief Defines the different configuration types that can be updated
             */
            enum class UpdateType {
                All       = 0x1F, /**< All configurations are updated */
                System    = 0x01, /**< The system settings are updated */
                Airports  = 0x02, /**< The airport settings are updated */
                Aircrafts = 0x04, /**< The aircraft settings are updated */
                Runtime   = 0x08, /**< The runtime settings are updated */
                Metar     = 0x10, /**< The METAR data is updated */
                Events    = 0x20  /**< The event route settings are updated */
            };

        private:
            std::mutex                                         m_configurationLock;
            types::SystemConfiguration                         m_systemConfig;
            types::RuntimeConfiguration                        m_runtimeConfig;
            types::EventRoutesConfiguration                    m_eventsConfig;
            std::map<std::string, formats::AirportFileFormat*> m_airportConfigurations;
            formats::AircraftFileFormat*                       m_aircraftConfiguration;
            std::map<void*, std::function<void(UpdateType)>>   m_notificationCallbacks;

            ConfigurationRegistry();
            void cleanup(UpdateType type);

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
             * @param[in] type The requested update type
             * @return True if the configuration is parsed, else false
             */
            bool configure(const std::string& path, UpdateType type);
            /**
             * @brief Returns the system configuration
             * @return The constant reference to the system configuration
             */
            const types::SystemConfiguration& systemConfiguration();
            /**
             * @brief Sets the new runtime configuration
             * @param[in] configuration The new runtime configuration
             */
            void setRuntimeConfiguration(const types::RuntimeConfiguration& configuration);
            /**
             * @brief Sets the new METAR information
             * @param[in] airport The airport's ICAO
             * @param[in] data The wind information
             */
            void setMetarInformation(const std::string& airport, const types::WindData& data);
            /**
             * @brief Returns the runtime configuration
             * @return The constant reference to the runtime configuration
             */
            const types::RuntimeConfiguration& runtimeConfiguration();
            /**
             * @brief Returns an airport configuration
             * @param[in] icao The airport's ICAO code
             * @return The airport configuration
             */
            const types::AirportConfiguration& airportConfiguration(const std::string& icao);
            /**
             * @brief Returns the parsed aircrafts
             * @return A map of aircrafts
             */
            const std::map<std::string, types::Aircraft>& aircrafts();
            /**
             * @brief Returns the event routes configuration
             * @return The event configuration
             */
            const types::EventRoutesConfiguration& eventRoutesConfiguration();
            /**
             * @brief Activates or deactivates an event
             * @param[in] event The event's name
             * @param[in] active True if the event is active, else false
             */
            void activateEvent(const std::string& event, bool active);
            /**
             * @brief Registers a callback that is triggered as soon as a new configuration is loaded
             * @tparam T The element which registers the callback
             * @tparam F The callback function
             * @param[in] instance The instance which registers the callback
             * @param[in] cbFunction The callback function
             */
            template <typename T, typename F>
            void registerNotificationCallback(T* instance, F cbFunction) {
                std::function<void(UpdateType)> func = std::bind(cbFunction, instance, std::placeholders::_1);
                this->m_notificationCallbacks[static_cast<void*>(instance)] = func;
            }
            /**
             * @brief Deletes a callback that is triggered as soon as a new configuration is loaded
             * @tparam T The element which registered the callback
             * @param[in] instance The instance which registers the callback
             */
            template <typename T>
            void deleteNotificationCallback(T* instance) {
                auto it = this->m_notificationCallbacks.find(static_cast<void*>(instance));
                if (this->m_notificationCallbacks.end() != it)
                    this->m_notificationCallbacks.erase(it);
            }
            /**
             * @brief Returns the global instance of the registry
             * @return The global instance
             */
            static ConfigurationRegistry& instance();
        };
    }
}
