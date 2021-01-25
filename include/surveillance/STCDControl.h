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
         *
         * The STCD module monitors all inbounds.
         * It uses the IPA or PRM-configuration of every airport to adapt the criteria of the separation evaluation.
         *
         * If the controller activates the IPA mode, the component calculates the NTZ, if the runways are allowed for
         * independent parallel approach.
         *
         * The system checks for every flight if it violates a NTZ or if the minimum required distance between two aircrafts is fulfilled.
         *
         * If an aircraft violates the NTZ, the system indicates it in the surveillance alert tag-entry via a "NTZ" message and the flag
         * is never removed until the aircraft is marked for a go-around.
         *
         * To estimate the separation minimums, the system finds the nearest aircraft that flies in front of the current one
         * and estimates the required minimum distance.
         * - In a very beginning is it checked, if the IPA is active or not.
         *   - If IPA is active does it define the minimum distance based on the WTCs of the preceding and following traffic,
         *     if both aircrafts are approaching the same runway
         *   - if IPA is inactive does it checks if both aircrafts are approaching different runways
         *      - If yes, a 3nm spacing is defined as the minimum
         *      - If no, it calculates the required spacing based on the WTCs of both flights
         */
        class STCDControl {
        private:
#ifndef DOXYGEN_IGNORE
            std::string                          m_airportIcao;
            types::Coordinate                    m_reference;
            std::list<types::Runway>             m_runways;
            std::list<types::SectorBorder>       m_noTransgressionZones;
            std::list<std::string>               m_ntzViolations;
            std::list<types::Flight>             m_inbounds;
            std::map<std::string, types::Length> m_conflicts;

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
            bool ntzViolation(const types::Flight& flight) const;
            bool separationLoss(const types::Flight& flight) const;
            const types::Length& minSeparation(const types::Flight& flight);
            /**
             * @brief Returns all defined NTZs
             * @return The NTZs
             */
            const std::list<types::SectorBorder>& noTransgressionZones() const;
#endif
        };
    }
}
