/*
 * @brief Defines a route of a flight plan
 * @file types/Route.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
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
