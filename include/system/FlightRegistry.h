/*
 * @brief Defines a flight registry
 * @file system/FlightRegistry.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <map>

#include <types/Flight.h>

namespace topskytower {
    namespace system {
        /**
         * @brief Describes a flight registry that contains all visible flights
         * @ingroup system
         */
        class FlightRegistry {
        private:
            std::map<std::string, std::pair<types::Flight, types::FlightPlan::AtcCommand>> m_flights;

            FlightRegistry();

        public:
            FlightRegistry(const FlightRegistry& other) = delete;
            FlightRegistry(FlightRegistry&& other) = delete;

            FlightRegistry& operator=(const FlightRegistry& other) = delete;
            FlightRegistry& operator=(FlightRegistry&& other) = delete;

            /**
             * @brief Updates or adds a flight
             * @param[in] flight The updated flight
             */
            void updateFlight(const types::Flight& flight);
            /**
             * @brief Removes a flight out of the registry
             * @param[in] callsign The flight's callsign
             */
            void removeFlight(const std::string& callsign);
            /**
             * @brief Checks if a flight for a specific callsign exists
             * @param[in] callsign The requested callsign
             * @return True if the flight exists, else false
             */
            bool flightExists(const std::string& callsign) const;
            /**
             * @brief Returns a specific flight
             * @param[in] callsign The requested callsign
             * @return The constant reference to the flight
             */
            const types::Flight& flight(const std::string& callsign) const;
            /**
             * @brief Overwrites the ATC clearance flag for a specific flight
             * @param[in] flight The updated flight
             * @param[in] flag The new clearance flag for departure and arrival
             */
            void setAtcClearanceFlag(const types::Flight& flight, std::uint16_t flag);
            /**
             * @brief Returns the flight registry instance
             * @return The registry
             */
            static FlightRegistry& instance();
        };
    }
}
