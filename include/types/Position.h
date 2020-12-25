/*
 * @brief Defines a geo-referenced coordinate with an altitude
 * @file types/Position.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <types/Coordinate.h>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a geo-reference coordinate and an altitude
         * @ingroup types
         */
        class Position {
        private:
            Coordinate m_coordinate;
            Length     m_altitude;
            Angle      m_heading;

        public:
            /**
             * @brief Creates a position with zero-values
             */
            Position();
            /**
             * @brief Creates a position with initial values
             * @param[in] coordinate The initial coordinate
             * @param[in] altitude The initial altitude
             * @param[in] heading The initial heading
             */
            Position(const Coordinate& coordinate, const Length& altitude, const Angle& heading);

            /**
             * @brief Returns the coordinate
             * @return The coordinate
             */
            const Coordinate& coordinate() const;
            /**
             * @brief Returns the altitude
             * @return The altitude
             */
            const Length& altitude() const;
            /**
             * @brief Returns the heading
             * @return The heading
             */
            const Angle& heading() const;
        };
    }
}
