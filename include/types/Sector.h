/*
 * @brief Defines a sector structure
 * @file types/Sector.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <list>
#include <string>

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
            std::string             m_identifier;
            Type                    m_type;
            std::string             m_prefix;
            std::string             m_midfix;
            std::string             m_suffix;
            std::string             m_frequency;
            std::list<SectorBorder> m_borders;

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
             * @brief Returns the sector's identifier
             * @return The identifier
             */
            const std::string& identifier() const;
            /**
             * @brief Returns the sector's type
             * @return The sector's type
             */
            Type type() const;
            /**
             * @brief Returns the prefix of the sector's login code
             * @return The sector's login code prefix
             */
            const std::string& prefix() const;
            /**
             * @brief Returns the midfix of the sector's login code
             * @return The sector's login code midfix
             */
            const std::string& midfix() const;
            /**
             * @brief Returns the suffix of the sector's login code
             * @return The sector's login code suffix
             */
            const std::string& suffix() const;
            /**
             * @brief Returns the sector's frequency
             * @return The sector's frequency
             */
            const std::string& frequency() const;
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
