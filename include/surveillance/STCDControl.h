/*
 * @brief Defines a STCD system
 * @file surveillance/STCDControl.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <list>
#include <map>

#include <system/ConfigurationRegistry.h>
#include <types/Flight.h>
#include <types/Runway.h>
#include <types/SectorBorder.h>

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Describes a Short Term Conflict Detection
         * @ingroup surveillance
         */
        class STCDControl {
        private:
#ifndef DOXYGEN_IGNORE
            std::string                    m_airportIcao;
            types::Coordinate              m_reference;
            std::list<types::Runway>       m_runways;
            std::list<types::SectorBorder> m_noTransgressionZones;

            void reinitialize(system::ConfigurationRegistry::UpdateType type);
            void createNTZ(const std::pair<std::string, std::string>& runwayPair);

        public:
            /**
             * @brief Creates a STCD control instance
             * @param[in] airport The airport's ICAO code
             * @param[in] center The reference point for the gnonomic transformation
             * @param[in] runways The runways of the airport
             */
            STCDControl(const std::string& airport, const types::Coordinate& center, const std::list<types::Runway>& runways);
            /**
             * @brief Destroys all internal structures and registrations
             */
            ~STCDControl();

            /**
             * @brief Updates a flight and calculates the metrices for this flight
             * @param[in] flight The updatable flight
             */
            void updateFlight(const types::Flight& flight);
            /**
             * @brief Removes the flight of this callsign
             * @param[in] callsign The flight's callsign
             */
            void removeFlight(const std::string& callsign);
            /**
             * @brief Returns all defined NTZs
             * @return The NTZs
             */
            const std::list<types::SectorBorder>& noTransgressionZones() const;
#endif
        };
    }
}
