/*
 * @brief Defines the base for all other configuration files in the EuroScope format
 * @file formats/EseFileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <map>

#include <types/Runway.h>
#include <types/Sector.h>

namespace topskytower {
    namespace formats {
        /**
         * @brief Defines the base parser of the euroscope configuration files
         * @ingroup format
         *
         * The ESE-file and all compatible files use sections between '[' and ']' brackets.
         * Each section is stored in the m_blocks-member and all coordinates are defined as following:
         * @code{.xml}
         * COORD:LATITUDE:LONGITUDE
         * @endcode
         */
        class EseFileFormat {
        private:
            std::list<types::SectorBorder>                  m_sectorBorders;
            std::list<types::Sector>                        m_sectors;
            std::map<std::string, std::list<types::Runway>> m_runways;

            void parseSectors(const std::vector<std::string>& positions, const std::vector<std::string>& airspace);
            void parseRunways(const std::vector<std::string>& runways);

        public:
            /**
             * @brief Creates an empty file format
             */
            EseFileFormat();
            /**
             * @brief Parses an ESE file and extracts all required information
             * If the file does not exist, the constructor throws an exeption.
             * @param[in] sectorName The sector's name which is used to find the sector file
             */
            EseFileFormat(const std::string& sectorName);

            /**
             * @brief Returns the sectors of the ESE file
             * @return The parsed sectors
             */
            const std::list<types::Sector>& sectors() const;
            /**
             * @brief Returns all parsed sector borders
             * @return The sector borders
             */
            const std::list<types::SectorBorder>& sectorBorders() const;
            /**
             * @brief Returns the runways of a specific airport
             * @param[in] airport The airport's ICAO code
             * @return The runways of the airport
             */
            const std::list<types::Runway>& runways(const std::string& airport) const;
        };
    }
}
