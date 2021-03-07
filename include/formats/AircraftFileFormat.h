/*
 * @brief Defines the aircraft file format
 * @file formats/AircraftFileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <map>

#include <formats/FileFormat.h>
#include <types/Aircraft.h>

namespace topskytower {
    namespace formats {
        /**
         * @brief Defines the parser of aircraft definitions
         * @ingroup format
         *
         * The aircraft file format specifies the aircrafts.
         * An aircraft contains relevant information about the dimension, weight, etc.
         * These information are required to assign stands correctly.
         *
         * An aircraft is defined in a single line in the following format:
         * @code{.xml}
         * ICAO:WINGSPAN:LENGTH:HEIGHT:MTOW:TYPE
         * @endcode
         *
         * The ICAO code is the official code of the aircraft.
         * The wingspan, length and height are defined in meters and the maximum take-off weight in kilograms.
         * The type differentiates between airline (A), military (M), business (B) and cargo (C).
         *
         * The aircraft file needs the following name: 'TopSkyTowerAircrafts.txt'.
         */
        class AircraftFileFormat : public FileFormat {
        private:
#ifndef DOXYGEN_IGNORE
            std::map<std::string, types::Aircraft> m_aircrafts;

            bool parseAircraft(const std::string& line);

        public:
            /**
             * @brief Parses an aircraft definition
             * If the file does not exist, the constructor throws an exeption.
             * @param[in] filename The filename of the configuration
             */
            explicit AircraftFileFormat(const std::string& filename);

            /**
             * @brief Returns the map of aircrafts
             * @return The aircrafts
             */
            const std::map<std::string, types::Aircraft>& aircrafts() const;
#endif
        };
    }
}
