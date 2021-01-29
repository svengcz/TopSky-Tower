/*
 * @brief Defines a waypoint of a route
 * @file types/Waypoint.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <string>

#include <types/Coordinate.h>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a waypoint of a route
         * @ingroup types
         */
        class Waypoint {
        private:
            std::string       m_name;
            types::Coordinate m_position;

        public:
            /**
             * @brief Creates an empty waypoint
             */
            Waypoint();
            /**
             * @brief Creates a waypoint
             * @param[in] name The waypoint's name
             * @param[in] position The waypoint's position
             */
            Waypoint(const std::string& name, const types::Coordinate& position);

            /**
             * @brief Returns the waypoint's name
             * @return The name
             */
            const std::string& name() const;
            /**
             * @brief Returns the waypoint's position
             * @return The position
             */
            const types::Coordinate& position() const;
        };
    }
}
