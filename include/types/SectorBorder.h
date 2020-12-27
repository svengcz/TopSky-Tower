/*
 * @brief Defines a sector border structure
 * @file types/SectorBorder.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>
#include <vector>

#pragma warning(push, 0)
#include <boost/geometry/geometries/polygon.hpp>
#pragma warning(pop)

#include <types/Coordinate.h>
#include <types/Position.h>
#include <types/Quantity.hpp>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a border of a controller sector
         * @ingroup types
         */
        class SectorBorder {
        private:
            std::string                    m_owner;
            std::vector<std::string>       m_deputies;
            types::Length                  m_lowerAltitude;
            types::Length                  m_upperAltitude;
            bg::model::polygon<Coordinate> m_shape;
            types::Angle                   m_boundingBox[2][2];

        public:
            /**
             * @brief Defines an unintialized border
             */
            SectorBorder() noexcept;
            /**
             * @brief Defines a new sector border with a deputy
             * @param[in] owner The sector's owner
             * @param[in] deputies The sector's deputies
             * @param[in] lowerAltitude The lower altitude of the sector
             * @param[in] upperAltitude The upper altitude of the sector
             */
            SectorBorder(std::string&& owner, std::vector<std::string>&& deputies, const types::Length& lowerAltitude,
                         const types::Length& upperAltitude) noexcept;
            /**
             * @Brief Moves other into this border
             * @param[in] other The source border
             */
            SectorBorder(const SectorBorder& other) noexcept;
            /**
             * @Brief Moves other into this border
             * @param[in] other The source border
             */
            SectorBorder(SectorBorder&& other) noexcept;

            /**
             * @brief Assigns other to this instance
             * @param[in] other The source instance
             * @return This instances with the copied values
             */
            SectorBorder& operator=(const SectorBorder& other) noexcept;
            /**
             * @brief Assigns other to this instance
             * @param[in] other The source instance
             * @return This instances with the moved values
             */
            SectorBorder& operator=(SectorBorder&& other) noexcept;

            /**
             * @brief Returns the owner of the sector border
             * @return The owner's identifier
             */
            const std::string& owner() const;
            /**
             * @brief Returns the deputies of the sector border
             * @return The deputies's identifier
             */
            const std::vector<std::string>& deputies() const;
            /**
             * @brief Returns the lower altitude of the border
             * @return The lower altitude
             */
            const types::Length& lowerAltitude() const;
            /**
             * @brief Returns the upper altitude of the border
             * @return The upper altitude
             */
            const types::Length& upperAltitude() const;
            /**
             * @brief Sets new edges of the border
             * @param[in] edges The edges of the border
             */
            void setEdges(const std::list<types::Coordinate>& edges);
            /**
             * @brief Returns the edges of the border
             * @return The edges that describe the border
             */
            const bg::model::ring<types::Coordinate>& edges() const;
            /**
             * @brief Checks if a coordinate is inside the border and ignores the altitude restrictions
             * @param[in] coordinate The checked coordinate
             * @return True if the coordinate is inside the border, else false
             */
            bool isInsideBorder(const types::Coordinate& coordinate) const;
            /**
             * @brief Checks if a position is inside the border
             * @param[in] position The checked position
             * @return True if the position is inside the border, else false
             */
            bool isInsideBorder(const types::Position& position) const;
        };
    }
}
