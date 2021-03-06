/*
 * @brief Defines a stand control system
 * @file management/StandControl.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <map>
#include <string>

#include <nanoflann.hpp>

#include <system/ConfigurationRegistry.h>
#include <types/AirportConfiguration.h>
#include <types/Flight.h>

namespace topskytower {
    namespace management {
        /**
         * @brief Describes a stand control system
         * @ingroup management
         *
         * The stand control assigns flights to stands and performs this on an extended configuration.
         * A stand defines constraints which kind of aircrafts are allowed to occupy the stand.
         * An airline reserves some stands with specific priorities and the system tries to assign the most important stand
         * to an arriving aircraft. The departing aircrafts are automatically assigned to the stand at which they are parked.
         * The assigned stands are colorized in a way that indicates if a stand needs to be published, is already blocked or is
         * correctly assigned to the aircraft.
         *
         * ![Stand assignments](doc/imgs/StandControl.png)
         *
         * A menu allows to automatically assign a new stand, manually assign a stand and publish the stand to other controllers.
         * The publish-function is compatible to GRplugin.
         */
        class StandControl {
        private:
#ifndef DOXYGEN_IGNORE
            struct StandData : public types::Stand {
                float                                                    cartesianPosition[2];
                std::list<std::pair<types::Flight, types::Flight::Type>> occupancyFlights;
                bool                                                     deactivated;

                StandData() :
                        cartesianPosition{ 0.0f, 0.0f },
                        occupancyFlights(),
                        deactivated(false) { }
            };

            struct StandTree {
                std::map<std::string, StandData> stands;

                StandTree() : stands() { }

                inline std::size_t kdtree_get_point_count() const {
                    return stands.size();
                }
                inline float kdtree_get_pt(const std::size_t idx, const std::size_t dimension) const {
                    auto it = this->stands.cbegin();
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
            types::Stand                                          m_gatPosition;

            static void copyStandData(const types::Stand& config, StandData& block);
            void reinitialize(system::ConfigurationRegistry::UpdateType type);
            void markStandAsOccupied(std::map<std::string, StandData>::iterator& iter, const types::Flight& flight,
                                     types::Flight::Type type);
            std::list<std::string> findAvailableAndUsableStands(const types::Flight& flight, bool ignoreManualFlag) const;
            bool findOptimalStand(const types::Flight& flight, types::Flight::Type type, const std::list<std::string>& availableStands);
            bool assignStand(const types::Flight& flight, types::Flight::Type type, const std::list<types::StandPriorities>& priorities,
                             const std::list<std::string>& availableStands);
            void notamsChanged();

        public:
            /**
             * @brief Creates a new stand control based on the current configuration
             * @param[in] airport The airport's ICAO code
             * @param[in] center The airport's center
             */
            StandControl(const std::string& airport, const types::Coordinate& center);
            /**
             * @brief Deletes all internal structures
             */
            ~StandControl();

            /**
             * @brief Updates a flight and assigns a stand if the aircraft is close enough
             * @param[in] flight The updatable flight
             * @param[in] type The flight's type
             */
            void updateFlight(const types::Flight& flight, types::Flight::Type type);
            /**
             * @brief Removes a flight out of the list
             * @param[in] callsign The deletable callsign
             */
            void removeFlight(const std::string& callsign);
            /**
             * @brief Assigns a stand manually to a flight
             * @param[in] flight The assignable flight
             * @param[in] type The flight's type
             * @param[in] stand The stand's identifier
             */
            void assignManually(const types::Flight& flight, types::Flight::Type type, const std::string& stand);
            /**
             * @brief Checks if a stand exists or not
             * @param[in] name The requested stand
             * @return True if the stand exists, else false
             */
            bool standExists(const std::string& name) const;
            /**
             * @brief Returns the assigned or occupied stand of a flight
             * @param[in] flight The requested flight
             * @return The stand's identifier or an empty string, if no stand is assigned
             */
            std::string stand(const types::Flight& flight) const;
            /**
             * @brief Returns the stand structure of a requested stand
             * @param[in] name The stand's name
             * @return The stand structure
             */
            const types::Stand& stand(const std::string& name) const;
            /**
             * @brief Returns all possible stands of the flight
             * @param[in] flight The requested flight
             * @return The list of all possible and available stands
             */
            std::list<std::string> allPossibleAndAvailableStands(const types::Flight& flight) const;
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
#endif
        };
    }
}
