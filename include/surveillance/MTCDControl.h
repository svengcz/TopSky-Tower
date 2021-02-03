/*
 * @brief Defines a MTCD system
 * @file surveillance/MTCDControl.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <functional>

#include <management/HoldingPointMap.h>
#include <surveillance/DepartureModel.h>
#include <types/Flight.h>

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Describes a Medium Term Conflict Detection system
         * @ingroup surveillance
         *
         * This module predicts departures that are close to active holding points.
         * Based on the predictions and the already departing flights is a Medium Term Conflict Detection implemented.
         *
         * The MTCD module helps to identify possible conflicts in the departure sector.
         * It uses the waypoint prediction of EuroScope, but uses it's own model to estimate the position and time when
         * reaching specific waypoints.
         *
         * The conflict analysis is based on the predicted routes, altitudes and speeds to check the criticality based
         * on the safety net configuration. It is possible to define the minimum required altitude difference
         * and the minimum required horizontal spacing. If multiple positions does not fit the safety configuration,
         * the most critical point is selected. The most critical point is defined by the time of occurence.
         *
         * The MTC-value is only calculated for non-departed flights, becaus departing flights cannot be handled
         * by the tower anymore. The system calculates the predicted route and all relevant metrices only
         * for aircrafts that are next to an active holding point and do not have a departure-clearance or
         * the ground speed is below 40 kn.
         *
         * The tag will be extended by the surveillance alert message "MTC".
         *
         * ![Medium Term Conflict](doc/imgs/MediumTermConflict.png)
         *
         * It is additionally possible to visualize the predicted conflict points for a specific aircraft.
         * All combinations between the specific aircraft and all other relevant aircrafts is visualized
         * with the information about the altitude difference, horizontal spacing and the time of occurence.
         *
         * ![Medium Term Conflict Position](doc/imgs/MTCDPrediction.png)
         *
         * An other visualizable information is the predicted path with the ETA for every waypoint on the SID,
         * The expected altitude and the expected speed. This helps to controller to verify the MTC-warning
         * and handle accordingly.
         *
         * ![Predicted route](doc/imgs/MTCDRoutePrediction.png)
         *
         * The following graphics shows the internal prediction model for IFR flights with a defined SID:
         *
         * ![Departure model](doc/imgs/DepartureModel.png)
         */
        class MTCDControl : management::HoldingPointMap<management::HoldingPointData> {
#ifndef DOXYGEN_IGNORE
        public:
            /**
             * @brief Defines a conflict between two flights
             */
            struct Conflict {
                std::string                      callsign; /**< The callsign of the other flight */
                DepartureModel::ConflictPosition position; /**< The conflict position */
            };

            typedef std::vector<types::Coordinate>(departureRoute)(const std::string&);

        private:
            std::function<departureRoute>              m_sidExtractionCallback;
            std::list<DepartureModel>                  m_departures;
            std::map<std::string, std::list<Conflict>> m_conflicts;

            void reinitialize(system::ConfigurationRegistry::UpdateType type);
            std::list<DepartureModel>::iterator insertFlight(const types::Flight& flight, types::Flight::Type type);
            void removeConflict(const std::string& callsignModel, const std::string& callsignConflict);

        public:
            /**
             * @brief Creates a MTCD control instance
             * @param[in] airport The airport's ICAO code
             * @param[in] center The airport's center position
             */
            MTCDControl(const std::string& airport, const types::Coordinate& center);
            /**
             * @brief Destroys all internal structures and registrations
             */
            ~MTCDControl();

            /**
             * @brief Updates a flight and calculates the MTCA metrices
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
             * @brief Checks if a departure model exists
             * @param[in] flight The requested flight
             * @return True if the departure model exists, else false
             */
            bool departureModelExists(const types::Flight& flight) const;
            /**
             * @brief Returns the departure model
             * @param[in] flight The requested flight
             * @return The departure model
             */
            const DepartureModel& departureModel(const types::Flight& flight) const;
            /**
             * @brief Returns the number of initiated conflicts of this flight
             * @param[in] flight The requested flight
             * @return True if conflicts exist, else false
             */
            bool conflictsExist(const types::Flight& flight) const;
            /**
             * @brief Returns the conflicts of a requested flight
             * @param[in] flight The requested flight
             * @return The requested conflicts
             */
            const std::list<Conflict>& conflicts(const types::Flight& flight) const;
            /**
             * @brief Registers a function that can be used to extract predicted SID points
             * The function helps to decrease the number of useless route extraction calls.
             * The assumption is that only a small number of flights requires predicted routes to perform the MTCA checks.
             *
             * The callback function requires one parameter:
             *  - The callsign of the requested flight
             * Afterwards does the callback return the prediction positions that ends with the given coordinate.
             *
             * @tparam T The element which registers the callback
             * @tparam F The callback function
             * @param[in] instance The instance which registers the callback
             * @param[in] cbFunction The callback function
             */
            template <typename T, typename F>
            void registerSidExtraction(T* instance, F cbFunction) {
                std::function<departureRoute> func = std::bind(cbFunction, instance, std::placeholders::_1);
                this->m_sidExtractionCallback = func;
            }
#endif
        };
    }
}
