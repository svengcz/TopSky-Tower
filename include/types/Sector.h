/*
 * @brief Defines a sector structure
 * @file types/Sector.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <list>
#include <string>

#include <types/ControllerInfo.h>
#include <types/SectorBorder.h>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a controller sector
         * @ingroup types
         */
        class Sector {
        public:
            /**
             * @brief Defines the different controller levels
             */
            enum class Type {
                Undefined     = 0, /**< Sector is owned by an undefined or unknown controller */
                ATIS          = 1, /**< Sector is owned by the ATIS */
                Delivery      = 2, /**< Sector is owned by a delivery controller */
                Ground        = 3, /**< Sector is owned by a ground controller */
                Tower         = 4, /**< Sector is owned by a tower controller */
                Departure     = 5, /**< Sector is owned by a departure controller */
                Approach      = 6, /**< Sector is owned by an approach controller */
                Center        = 7, /**< Sector is owned by a center controller */
                FlightService = 8  /**< Sector is owned by a flight service controller */
            };

        private:
            ControllerInfo          m_info;
            Type                    m_type;
            std::list<SectorBorder> m_borders;

            void parseSectorType();

        public:
            /**
             * @brief Creates an unintialized sector
             */
            Sector();
            /**
             * @brief Creates a new sector
             * @param[in] identifier The controller's identifier
             * @param[in] prefix The controller's prefix
             * @param[in] midfix The controller's midfix
             * @param[in] suffix The controller's suffix
             * @param[in] frequency The controller's frequency
             */
            Sector(std::string&& identifier, std::string&& prefix, std::string&& midfix,
                   std::string&& suffix, std::string&& frequency);
            /**
             * @brief Creates a new sector
             * @param[in] identifier The controller's identifier
             * @param[in] prefix The controller's prefix
             * @param[in] midfix The controller's midfix
             * @param[in] suffix The controller's suffix
             * @param[in] frequency The controller's frequency
             * @param[in] latitude The sector's latitude of the center point
             * @param[in] longitude The sector's longitude of the center point
             */
            Sector(std::string&& identifier, std::string&& prefix, std::string&& midfix,
                   std::string&& suffix, std::string&& frequency, const std::string& latitude,
                   const std::string& longitude);
            /**
             * @brief Moves one sector into the other
             * @param[in] other The source sector
             */
            Sector(const Sector& other) noexcept;
            /**
             * @brief Moves one sector into the other
             * @param[in] other The source sector
             */
            Sector(Sector&& other) noexcept;

            /**
             * @brief Assigns other to this instance
             * @param[in] other The source instance
             * @return This instances with the copied values
             */
            Sector& operator=(const Sector& other) noexcept;
            /**
             * @brief Assigns other to this instance
             * @param[in] other The source instance
             * @return This instances with the moved values
             */
            Sector& operator=(Sector&& other) noexcept;

            /**
             * @brief Returns the sector's type
             * @return The sector's type
             */
            Type type() const;
            /**
             * @brief Returns the controller information
             * @return The controller information
             */
            const ControllerInfo& controllerInfo() const;
            /**
             * @brief Sets the borders of the sector
             * The borders are sorted by the maximum altitude of the borders from lower to higher.
             * @param[in] borders The borders
             */
            void setBorders(std::list<SectorBorder>&& borders);
            /**
             * @brief Returns the borders of the sector
             * @return The borders
             */
            const std::list<SectorBorder>& borders() const;
            /**
             * @brief Checks if a coordinate is inside one of the sector's borders
             * @param[in] coordinate The checked coordinate
             * @return True if the coordinate is inside one of the borders, else false
             */
            bool isInsideSector(const types::Coordinate& coordinate) const;
            /**
             * @brief Checks if a position is inside one of the sector's borders
             * @param[in] position The checked position
             * @return True if the position is inside one of the borders, else false
             */
            bool isInsideSector(const types::Position& position) const;
        };
    }
}
