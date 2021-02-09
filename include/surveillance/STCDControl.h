/*
 * @brief Defines a STCD system
 * @file surveillance/STCDControl.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <list>
#include <map>

#include <management/DepartureSequenceControl.h>
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
            using TimePoint = std::chrono::system_clock::time_point;

            struct Departure {
                std::string          callsign;
                types::Aircraft::WTC wtc;
                TimePoint            departureTime;

                Departure() :
                        callsign(),
                        wtc(types::Aircraft::WTC::Unknown),
                        departureTime() { }
            };

            std::string                           m_airportIcao;
            types::Length                         m_airportElevation;
            types::Coordinate                     m_reference;
            management::DepartureSequenceControl* m_departureControl;
            std::list<types::Runway>              m_runways;
            std::list<types::SectorBorder>        m_noTransgressionZones;
            std::list<std::string>                m_ntzViolations;
            std::list<types::Flight>              m_inbounds;
            std::map<std::string, types::Length>  m_conflicts;

            void reinitialize(system::ConfigurationRegistry::UpdateType type);
            void createNTZ(const std::pair<std::string, std::string>& runwayPair);
            void analyzeInbound(const types::Flight& flight);
            void analyzeOutbound(const types::Flight& flight);

        public:
            /**
             * @brief Creates a STCD control instance
             * @param[in] airport The airport's ICAO code
             * @param[in] elevation The airport's elevation
             * @param[in] center The reference point for the gnonomic transformation
             * @param[in] runways The runways of the airport
             * @param[in] departureControl The departure sequence control system
             */
            STCDControl(const std::string& airport, const types::Length& elevation, const types::Coordinate& center,
                        const std::list<types::Runway>& runways, management::DepartureSequenceControl* departureControl);
            /**
             * @brief Destroys all internal structures and registrations
             */
            ~STCDControl();

            /**
             * @brief Updates a flight and calculates the metrices for this flight
             * @param[in] flight The updatable flight
             * @param[in] type The flight's type
             */
            void updateFlight(const types::Flight& flight, types::Flight::Type type);
            /**
             * @brief Removes the flight of this callsign
             * @param[in] callsign The flight's callsign
             */
            void removeFlight(const std::string& callsign);
            /**
             * @brief Checks if the NTZ was violated
             * @param[in] flight The requested flight
             * @return True if the NTZ was violated, else false
             */
            bool ntzViolation(const types::Flight& flight) const;
            /**
             * brief Checks if the separation to the preceding traffic is lost
             * @param[in] flight The requested flight
             * @return True if the separation is lost, else false
             */
            bool separationLoss(const types::Flight& flight) const;
            /**
             * @brief Returns the minimum required separation
             * @param[in] flight The requested flight
             * @return The minimum required separation
             */
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
