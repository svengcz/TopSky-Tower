/*
 * @brief Defines a CMAC system
 * @file surveillance/CMACControl.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <map>

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
         * ![Conformance Monitoring Alert](doc/imgs/ConformanceMonitoringAlert.png)
         */
        class CMACControl {
        private:
#ifndef DOXYGEN_IGNORE
            struct FlightHistory {
                std::size_t                   cycleCounter;
                types::Coordinate             referencePosition;
                types::FlightPlan::AtcCommand expectedCommand;
            };

            std::map<std::string, FlightHistory> m_tracks;

        public:
            /**
             * @brief Creates a CMAC control instance
             */
            CMACControl();

            /**
             * @brief Updates a flight and calculates the ARIWS metrices
             * @param[in] flight The updated flight
             */
            void updateFlight(const types::Flight& flight);
            /**
             * @brief Removes a flight out of the internal system
             * @param[in] callsign The removable callsign
             */
            void removeFlight(const std::string& callsign);
            /**
             * @brief Checks if a flight is marked as CMA relevant
             * @param[in] flight The requested flight
             * @return True if CMA is valid, else false
             */
            bool conformanceMonitoringAlert(const types::Flight& flight) const;
#endif
        };
    }
}
