/*
 * @brief Defines a ARIWS system
 * @file surveillance/ARIWSControl.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <vector>

#include <management/HoldingPointMap.h>
#include <system/ConfigurationRegistry.h>
#include <types/AirportConfiguration.h>
#include <types/Flight.h>

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Describes an Autonomous Runway Incursion Warning System
         * @ingroup surveillance
         *
         * The ARIWS is used to identify flights that are joining a runway without any clearance.
         * The system uses the extended Ground status flags to check if the flight is departing or lining up.
         *
         * The system is designed in that way that it is a bit more relaxed than the realworld application.
         * It defines some dead area around a holding point where an aircraft can hold position without a RIW trigger.
         * This is used to reduce the number of false-positives due to inaccurate taxiings or scenery mismatches.
         *
         * If an aircraft is behind a configured holding point, facing into the runway's direction and does not have
         * a line-up or departure clearance, the RIW is triggered and visualized in the "TopSky-Tower / Surveillance alerts" tag element.
         *
         * ![Runway Incursion Warning](doc/imgs/RunwayIncursionWarning.png)
         */
        class ARIWSControl : management::HoldingPointMap<management::HoldingPointData> {
        private:
#ifndef DOXYGEN_IGNORE
            std::list<std::string> m_incursionWarnings;

            void reinitialize(system::ConfigurationRegistry::UpdateType type);

        public:
            /**
             * @brief Creates a ARIWS control instance
             * @param[in] airport The airport's ICAO code
             * @param[in] center The airport's center position
             */
            ARIWSControl(const std::string& airport, const types::Coordinate& center);
            /**
             * @brief Deletes all internal structures
             */
            ~ARIWSControl();

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
             * @brief Checks if a flight is marked as runway incursion
             * @param[in] flight The requested flight
             * @return True if RIW is valid, else false
             */
            bool runwayIncursionWarning(const types::Flight& flight) const;
#endif
        };
    }
}
