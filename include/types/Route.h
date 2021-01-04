/*
 * @brief Defines a route of a flight plan
 * @file types/Route.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <vector>

#include <types/Waypoint.h>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a route which contains all waypoints
         * @ingroup types
         */
        class Route {
        private:
            std::vector<Waypoint> m_waypoints;

        public:
            /**
             * @brief Creates an empty route
             */
            Route();
            /**
             * @brief Creates a route with defined waypoints
             * @param[in] waypoints All waypoints of the route
             */
            Route(std::vector<Waypoint>&& waypoints);

            /**
             * @brief Returns the waypoints
             * @return The waypoints
             */
            const std::vector<Waypoint>& waypoints() const;
        };
    }
}
