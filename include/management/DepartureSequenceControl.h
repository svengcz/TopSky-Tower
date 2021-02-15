/*
 * @brief Defines a departure sequencing control system
 * @file management/DepartureSequenceControl.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <chrono>
#include <map>

#include <management/HoldingPointMap.h>
#include <types/Aircraft.h>

namespace topskytower {
    namespace management {
        /**
         * @brief The departure sequence control tracks all departure ready flights at a holding point or by a marker
         * @ingroup management
         */
        class DepartureSequenceControl {
#ifndef DOXYGEN_IGNORE
        private:
            using TimePoint = std::chrono::system_clock::time_point;

            struct DepartureInformation {
                std::string          callsign;
                bool                 reachedHoldingPoint;
                bool                 passedHoldingPoint;
                bool                 normalProcedureHoldingPoint;
                types::HoldingPoint  holdingPoint;
                types::Aircraft::WTC wtc;
                TimePoint            actualTakeOffTime;
                types::Coordinate    lastReportedPosition;
                types::Length        flewDistance;
            };

            std::string                                 m_airport;
            HoldingPointMap<HoldingPointData>           m_holdingPoints;
            std::map<std::string, DepartureInformation> m_departureReady;
            std::map<std::string, DepartureInformation> m_departedPerRunway;

            void reinitialize(system::ConfigurationRegistry::UpdateType type);

        public:
            /**
             * @brief Creates a departure sequence control instance
             * @param[in] airport The airport's ICAO code
             * @param[in] center The reference point for the gnonomic transformation
             */
            DepartureSequenceControl(const std::string& airport, const types::Coordinate& center);

            /**
             * @brief Updates a flight and checks if it is one of the next departure candidates
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
             * @brief Returns all flights that are ready for departure
             * @return All flights that are fully ready
             */
            std::list<std::string> allReadyForDepartureFlights() const;
            /**
             * @brief Returns the holding point candidates of this flight
             * @param[in] flight The requested flight
             * @return The list of holding points candidates
             */
            std::list<types::HoldingPoint> holdingPointCandidates(const types::Flight& flight) const;
            /**
             * @brief Checks if a flight is ready for departure
             * @param[in] flight The requested flight
             * @return True if it is ready for departure, else false
             */
            bool readyForDeparture(const types::Flight& flight) const;
            /**
             * @brief Checks if a holding point is assigned to the flight
             * @param[in] flight The requested flight
             * @return True if a holding point exists, else false
             */
            bool hasHoldingPoint(const types::Flight& flight) const;
            /**
             * @brief Returns the assigned holding point
             * @param[in] flight The requested flight
             * @return The holding point
             */
            const types::HoldingPoint& holdingPoint(const types::Flight& flight) const;
            /**
             * @brief Sets the new holding point for a specific flights
             * @param[in] flight The requested flight
             * @param[in] name The holding point's name
             */
            void setHoldingPoint(const types::Flight& flight, const std::string& name);
            /**
             * @brief Returns the required spacings between the requested flight and the last departed one
             * @param[in] flight The requested flight
             * @param[out] timeSpacing The remaining time spacing
             * @param[out] separation The remaining separation
             */
            void departureSpacing(const types::Flight& flight, types::Time& timeSpacing, types::Length& separation) const;
#endif
        };
    }
}
