/*
 * @brief Defines a runway
 * @file types/Runway.h
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
         * @brief Describes a runway
         * @ingroup types
         */
        class Runway {
        private:
            std::string       m_name;
            types::Coordinate m_start;
            types::Coordinate m_end;
            types::Angle      m_heading;
            types::Length     m_length;

        public:
            /**
             * @brief Creates an empty runway
             */
            Runway();
            /**
             * @brief Creates a runway
             * @param[in] name The runway's name
             * @param[in] start The runway's start position
             * @param[in] end The runway's end position
             */
            Runway(const std::string& name, const types::Coordinate& start, const types::Coordinate& end);

            /**
             * @brief Returns the runway's name
             * @return The name
             */
            const std::string& name() const;
            /**
             * @brief Returns the runway's start position
             * @return The start position
             */
            const types::Coordinate& start() const;
            /**
             * @brief Returns the runway's end position
             * @return The end position
             */
            const types::Coordinate& end() const;
            /**
             * @brief Returns the runway's heading
             * @return The heading
             */
            const types::Angle& heading() const;
            /**
             * @brief Returns the runway's length
             * @return The length
             */
            const types::Length& length() const;
        };
    }
}
