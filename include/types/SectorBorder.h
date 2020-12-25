/*
 * @brief Defines a sector border structure
 * @file types/SectorBorder.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>
#include <vector>

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
        public:
            /**
             * @brief Defines a line segment of the border
             * The line segment is defined by the start and end point.
             * The mathematical description is 0 = a * x + b * y + c
             */
            struct LineSegment {
                types::Coordinate points[2];     /**< Start and end point of the segment */
                float             parameters[3]; /**< The three parameters (a, b, c) for the line form */
            };

        private:
            std::string                    m_owner;
            std::vector<std::string>       m_deputies;
            types::Length                  m_lowerAltitude;
            types::Length                  m_upperAltitude;
            std::vector<types::Coordinate> m_edges;
            std::vector<LineSegment>       m_segments;
            types::Angle                   m_boundingBox[2][2];

            static void calculateLineParameters(LineSegment& segment);
            static bool intersects(const LineSegment& segment0, const LineSegment& segment1);

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
             * @brief Inserts anew edge into the border
             * @param[in] coord0 The first coordinate of the edge
             * @param[in] coord1 The second coordinate of the edge
             */
            void addEdge(const types::Coordinate& coord0, const types::Coordinate& coord1);
            /**
             * @brief Returns the edges of the border
             * @return The edges that describe the border
             */
            const std::vector<types::Coordinate>& edges() const;
            const std::vector<LineSegment>& segments() const;
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
