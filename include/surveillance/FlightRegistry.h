/*
 * @brief Defines a flight registry
 * @file surveillance/FlightRegistry.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <map>

#include <types/Flight.h>

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Describes a flight registry that contains all visible flights
         * @ingroup surveillance
         */
        class FlightRegistry {
        private:
            std::map<std::string, types::Flight> m_flights;

        public:
            /**
             * @brief Creates an empty flight registry
             */
            FlightRegistry();

            /**
             * @brief Updates or adds a flight
             * @param[in] flight The updated flight
             */
            void updateFlight(types::Flight&& flight);
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
        };
    }
}