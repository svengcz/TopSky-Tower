/*
 * @brief Defines a flight model for departures and adapts performances based on measurements
 * @file surveillance/DepartureModel.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#ifndef DOXYGEN_IGNORE

#include <chrono>
#include <vector>

#pragma warning(push, 0)
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometry.hpp>
#pragma warning(pop)

#include <types/Flight.h>

namespace bg = boost::geometry;

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Defines a flight model for departures
         * @ingroup surveillance
         */
        class DepartureModel {
        public:
            /**
             * @brief Defines a single waypoint with speed and time information
             */
            struct Waypoint {
                types::Position position;   /**< Defines the position of the waypoint with the predicted altitude */
                types::Velocity speed;      /**< Defines the predicted speed at this point */
                types::Time     reachingIn; /**< Defines the predicted time when the waypoint is reached */
            };

            /**
             * @brief Defines a conflict position of two different departure routes
             */
            struct ConflictPosition {
                types::Coordinate coordinate;         /**< Defines the coordinate of the conflict */
                types::Length     altitudeDifference; /**< Defines the altitude difference between both flights */
                types::Length     horizontalSpacing;  /**< Defines the horizontal spacing between both points */
            };

        private:
            using TimePoint = std::chrono::system_clock::time_point;

            enum class Phase {
                TakeOff              = 0,
                AccelerationAltitude = 1,
                AccelerationFL100    = 2,
                ClimbFL100           = 3,
                AccelerationCruise   = 4,
                ClimbCruise          = 5
            };

            types::Flight                                                        m_flight;
            types::Coordinate                                                    m_reference;
            TimePoint                                                            m_lastUpdate;
            Phase                                                                m_currentPhase;
            types::Velocity                                                      m_v2Speed;
            types::Velocity                                                      m_climbRate;
            types::Velocity                                                      m_climbRateAcceleration;
            types::Acceleration                                                  m_acceleration;
            types::Velocity                                                      m_cruiseSpeed;
            std::vector<Waypoint>                                                m_waypoints;
            bg::model::linestring<bg::model::point<float, 2, bg::cs::cartesian>> m_routeCartesian;

            Phase identifyPhase(const types::Length& altitude, const types::Velocity& speed,
                                const types::Velocity& climbRate) const;
            types::Length predictNextPhase(const types::Velocity& speed0, const types::Length& altitude0,
                                           Phase currentPhase, types::Velocity& speed1, types::Length& altitude1,
                                           types::Time& requiredTime) const;
            Waypoint predictWaypoint(const Waypoint& waypoint, const types::Coordinate& destination) const;
            void predictWaypoints(const std::vector<types::Coordinate>& waypoints);
            static bool findSegment(const std::vector<Waypoint>& route, const types::Coordinate& point,
                                    std::size_t& startIdx, std::size_t& endIdx);

        public:
            /**
             * @brief Creates a new departure model without the initialization of internal values
             * @param[in] callsign The flights callsign
             */
            explicit DepartureModel(const std::string& callsign);
            /**
             * @brief Creates a new departure model
             * @param[in] flight The trackable flight
             * @param[in] reference The reference point for the projections
             * @param[in] waypoints The predicted route
             */
            DepartureModel(const types::Flight& flight, const types::Coordinate& reference,
                           const std::vector<types::Coordinate>& waypoints);

            /**
             * @brief Defines the equal-compare-operator
             * @param[in] other The other model
             * @return True if the tracked flights are equal, else false
             */
            bool operator==(const DepartureModel& other) const;
            /**
             * @brief Defines the non-equal-compare-operator
             * @param[in] other The other model
             * @return True if the tracked flights are non-equal, else false
             */
            bool operator!=(const DepartureModel& other) const;

            /**
             * @brief Updates the internal states and predicts the new waypoints
             * @param[in] flight The updated flight
             * @param[in] waypoints The new waypoints
             */
            void update(const types::Flight& flight, const std::vector<types::Coordinate>& waypoints);
            /**
             * @brief Finds all conflict candidate posititions between two different departure models
             * @param[in] other The comparable departure model
             * @return All detected conflict candidates
             */
            std::list<ConflictPosition> findConflictCandidates(const DepartureModel& other) const;
            /**
             * @brief Returns the flight infmoration of this departure
             * @return The reference to the flight
             */
            const types::Flight& flight() const;
            /**
             * @brief Returns the predicted waypoints
             */
            const std::vector<Waypoint>& waypoints();
        };
    }
}

#endif
