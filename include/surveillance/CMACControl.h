/*
 * @brief Defines a CMAC system
 * @file surveillance/CMACControl.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <map>

#include <management/HoldingPointMap.h>
#include <types/Flight.h>

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Describes a Conformance Monitoring Alerts for Controllers System
         * @ingroup surveillance
         *
         * This module controls all kind of ground movements.
         * It tracks the different flights and estimates based on the position updates which kind of movement is performed.
         * If the aircraft moves backwards, indicated by the position and the aircraft's heading, it expects the PUSH-clearance to be set
         * or if the aircraft taxies forward, a TAXI-, LI-UP- or DEPA-clearance is expected.
         *
         * If the set ground status command does not fit to the expected one, the CMA is triggered and the "TopSky-Tower / Surveillance alerts"-tag
         * is extended by the CMA message.
         *
         * The system supervises the arrival flights as well.
         * It uses the defined holding points to check if the flight left the runway or is still vacating.
         *
         * ![Conformance Monitoring Alert](doc/imgs/ConformanceMonitoringAlert.png)
         */
        class CMACControl {
        private:
#ifndef DOXYGEN_IGNORE
            struct FlightHistory {
                std::size_t                   cycleCounter;
                types::Coordinate             referencePosition;
                bool                          behindHoldingPoint;
                types::FlightPlan::AtcCommand expectedCommand;

                FlightHistory() :
                        cycleCounter(0),
                        referencePosition(),
                        behindHoldingPoint(false),
                        expectedCommand(types::FlightPlan::AtcCommand::Unknown) { }
            };

            management::HoldingPointMap<management::HoldingPointData> m_holdingPoints;
            std::map<std::string, FlightHistory>                      m_tracks;

            void reinitialize(system::ConfigurationRegistry::UpdateType type);

        public:
            /**
             * @brief Creates a CMAC control instance
             * @param[in] airport The airport's ICAO code
             * @param[in] center The airport's center position
             */
            CMACControl(const std::string& airport, const types::Coordinate& center);
            /**
             * @brief Destroys all internal structures
             */
            ~CMACControl();

            /**
             * @brief Updates a flight and calculates the ARIWS metrices
             * @param[in] flight The updated flight
             * @param[in] type The flight's type
             */
            void updateFlight(const types::Flight& flight, types::Flight::Type type);
            /**
             * @brief Removes a flight out of the internal system
             * @param[in] callsign The removable callsign
             */
            void removeFlight(const std::string& callsign);
            /**
             * @brief Checks if a flight is marked as CMA relevant
             * @param[in] flight The requested flight
             * @param[in] type The flight's type
             * @return True if CMA is valid, else false
             */
            bool conformanceMonitoringAlert(const types::Flight& flight, types::Flight::Type type) const;
#endif
        };
    }
}
