/*
 * @brief Defines a MTCD system
 * @file surveillance/MTCDControl.h
 * @author Sven Czarnian <devel@svcz.de>
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
         */
        class MTCDControl : management::HoldingPointMap<management::HoldingPointData> {
#ifndef DOXYGEN_IGNORE
        public:
            /**
             * @brief Defines a conflict between two flights
             */
            struct Conflict {
                std::string                      callsigns[2]; /**< The conflicting callsigns */
                DepartureModel::ConflictPosition position;     /**< The conflict position */
            };

            typedef std::vector<types::Coordinate>(departureRoute)(const std::string&);

        private:
            std::function<departureRoute> m_sidExtractionCallback;
            std::list<DepartureModel>     m_departures;
            std::list<Conflict>           m_conflicts;

            void reinitialize(system::ConfigurationRegistry::UpdateType type);
            std::list<DepartureModel>::iterator insertFlight(const types::Flight& flight);

        public:
            /**
             * @brief Creates a MTCD control instance
             * @param[in] airport The airport's ICAO code
             * @param[in] center The airport's center position
             */
            MTCDControl(const std::string& airport, const types::Coordinate& center);

            /**
             * @brief Updates a flight and calculates the MTCA metrices
             * @param[in] flight The updated flight
             */
            void updateFlight(const types::Flight& flight);
            /**
             * @brief Removes a flight out of the internal system
             * @param[in] callsign The removable callsign
             */
            void removeFlight(const std::string& callsign);
            /**
             * @brief Returns the number of initiated conflicts of this flight
             * @param[in] flight The requested flight
             * @return The number of conflicts
             */
            std::size_t numberOfConflicts(const types::Flight& flight) const;
            /**
             * @brief Returns a conflict of a requested flight
             * @param[in] flight The requested flight
             * @param[in] idx The requested conflict index (needs to be smaller than numberOfConflicts)
             * @return The requested conflict
             */
            const Conflict& conflict(const types::Flight& flight, std::size_t idx) const;
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
