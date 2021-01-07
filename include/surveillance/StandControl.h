/*
 * @brief Defines a stand control system
 * @file surveillance/StandControl.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <map>
#include <string>

#include <nanoflann.hpp>

#include <system/ConfigurationRegistry.h>
#include <types/AirportConfiguration.h>
#include <types/Flight.h>

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Describes a stand control system
         * @ingroup surveillance
         */
        class StandControl {
        private:
            struct StandData : public types::Stand {
                float                    cartesianPosition[2];
                std::list<types::Flight> occupancyFlights;
            };
            struct StandTree {
                std::map<std::string, StandData> stands;

                inline std::size_t kdtree_get_point_count() const {
                    return stands.size();
                }
                inline float kdtree_get_pt(const std::size_t idx, const std::size_t dimension) const {
                    auto it = stands.cbegin();
                    std::advance(it, idx);
                    return it->second.cartesianPosition[dimension];
                }
                template <class BBOX>
                bool kdtree_get_bbox(BBOX&) const {
                    return false;
                }
            };
            typedef nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<float, StandTree>,
                                                        StandTree, 2> StandTreeAdaptor;

            std::string                                           m_airportIcao;
            StandTree                                             m_standTree;
            StandTreeAdaptor*                                     m_standTreeAdaptor;
            std::map<std::string, std::string>                    m_aircraftStandRelation;
            types::Coordinate                                     m_centerPosition;
            std::map<std::string, types::AirlineStandAssignments> m_standPriorities;

            static void copyStandData(const types::Stand& config, StandData& block);
            void reinitialize(system::ConfigurationRegistry::UpdateType type);
            void markStandAsOccupied(std::map<std::string, StandData>::iterator& iter, const types::Flight& flight);
            std::list<std::string> findAvailableAndUsableStands(const types::Flight& flight) const;
            bool findOptimalStand(const types::Flight& flight, const std::list<std::string>& availableStands);
            bool assignStand(const types::Flight& flight, const std::list<types::StandPriorities>& priorities,
                             const std::list<std::string>& availableStands);

        public:
            /**
             * @brief Creates a new stand control based on the current configuration
             * @param[in] airport The airport's ICAO code
             */
            StandControl(const std::string& airport);
            /**
             * @brief Deletes all internal structures
             */
            ~StandControl();

            /**
             * @brief Updates a flight and assigns a stand if the aircraft is close enough
             * @param[in] flight The updatable flight
             */
            void updateFlight(const types::Flight& flight);
            /**
             * @brief Removes a flight out of the list
             * @param[in] callsign The deletable callsign
             */
            void removeFlight(const std::string& callsign);
            /**
             * @brief Assigns a stand manually to a flight
             * @param[in] flight The assignable flight
             * @param[in] stand The stand's identifier
             */
            void assignManually(const types::Flight& flight, const std::string& stand);
            /**
             * @brief Returns the assigned or occupied stand of a flight
             * @param[in] flight The requested flight
             * @return The stand's identifier or an empty string, if no stand is assigned
             */
            std::string stand(const types::Flight& flight) const;
            /**
             * @brief Returns a list of all stand with a flag that indicates if the stand is occupied or not
             * @return The list of all stands with the occupation state
             */
            std::list<std::pair<std::string, bool>> allStands() const;
            /**
             * @brief Returns if the stand is blocked by an other flight
             * @param[in] stand The requested stand
             * @return True if an other flight is associated to the stand, else false
             */
            bool standIsBlocked(const std::string& stand) const;
        };
    }
}
