/*
 * @brief Defines the base for all other configuration files in the EuroScope format
 * @file configuration/EuroscopeFileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include <formats/FileFormat.h>
#include <types/Coordinate.h>

namespace topskytower {
    namespace formats {
        /**
         * @brief Defines the base parser of INI like file formats
         * @ingroup format
         *
         * The INI-file and all compatible files use sections between '[' and ']' brackets.
         * Each section is stored in the m_blocks-member and all coordinates are defined as following:
         * @code{.xml}
         * COORD:LATITUDE:LONGITUDE
         * @endcode
         */
        class IniFileFormat : public FileFormat {
        public:
            std::map<std::string, std::vector<std::string>> m_blocks;      /**< Contains all read sections without empty lines */
            std::map<std::string, std::uint32_t>            m_lineOffsets; /**< Contains the line offsets of the different blocks */

            /**
             * @brief Parses filename and splits the sections into the member m_blocks
             *
             * If the file does not exist, the constructor throws an exeption.
             *
             * @param[in] filename The filename to the file
             */
            IniFileFormat(const std::string& filename);

            /**
             * @brief Parses a coordinate entry in the file and returns the coordinate
             * @param[in] line The line with the COORD tag
             * @return The parsed coordinate
             */
            static types::Coordinate parsePosition(const std::string& line);
        };
    }
}
