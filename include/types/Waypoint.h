/*
 * @brief Defines a waypoint of a route
 * @file types/Waypoint.h
 * @author Sven Czarnian <devel@svcz.de>
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
            Waypoint(const std::string& name, const types::Coordinate& position);

            const std::string& name() const;
            const types::Coordinate& position() const;
        };
    }
}
