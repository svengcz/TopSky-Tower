/*
 * @brief Defines a geo-referenced coordinate
 * @file types/Coordinate.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <string>

#include <types/Quantity.hpp>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a geo-reference coordinate with longitude and latitude
         * @ingroup types
         */
        class Coordinate {
        private:
            Angle m_longitude;
            Angle m_latitude;

        public:
            /**
             * @brief Creates a coordinate with zero-values
             */
            Coordinate();
            /**
             * @brief Creates a coordinate with initial values
             * @param[in] longitude The initial longitudinal value
             * @param[in] latitude The initial latitudinal value
             */
            Coordinate(const Angle& longitude, const Angle& latitude);
            /**
             * @brief Creates a coordinate with initial values
             * The format of the coordinate component needs to be the followin:
             * - Longitude: [N,S]DEGREE.MINUTES.SECONDS.FRACTION
             * - Latitude: [E,W]DEGREE.MINUTES.SECONDS.FRACTION
             * @param[in] longitude The initial longitudinal value
             * @param[in] latitude The initial latitudinal value
             */
            Coordinate(const std::string& longitude, const std::string& latitude);

            /**
             * @brief Checks if two positions are equal
             * @param[in] other The other coordinate instance
             * @return True if this instance is equal to the other instance, else false
             */
            bool operator==(const Coordinate& other) const;
            /**
             * @brief Checks if two positions are not equal
             * @param[in] other The other coordinate instance
             * @return True if this instance is not equal to the other instance, else false
             */
            bool operator!=(const Coordinate& other) const;

            /**
             * @brief Returns the longitudinal component
             * @return The constant reference to the longitudinal component
             */
            const Angle& longitude() const;
            /**
             * @brief Returns the longitudinal component
             * @return The reference to the longitudinal component
             */
            Angle& longitude();
            /**
             * @brief Returns the latitudinal component
             * @return The constant reference to the latitudinal component
             */
            const Angle& latitude() const;
            /**
             * @brief Returns the latitudinal component
             * @return The reference to the latitudinal component
             */
            Angle& latitude();
            /**
             * @brief Calculates the coordinate from this coordinate and based on a heading and a distance
             * The function uses the haversine-formular and calculates the coordinate based on the great circle distance
             * @param[in] heading The heading from this coordinate to the next one
             * @param[in] distance The distance from this coordinate to the next one
             * @return The resulting coordinate which is based on the heading and the distance
             */
            Coordinate projection(const Angle& heading, const Length& distance) const;
            /**
             * @brief Calculates the great circle distance between this coordinate and the other
             * @param[in] other The other coordinate
             * @return The great circle distance between this coordinate and the other
             */
            Length distanceTo(const Coordinate& other) const;
            /**
             * @brief Calculates the bearing between this coordinate and the other
             * @param[in] other The other coordinate
             * @return The bearing between this coordinate and the other
             */
            Angle bearingTo(const Coordinate& other) const;
        };
    }
}
