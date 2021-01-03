/*
 * @brief Defines the aircraft file format
 * @file configuration/AircraftFileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <map>

#include <types/Aircraft.h>

namespace topskytower {
    namespace formats {
        /**
         * @brief Defines the parser of aircraft definitions
         * @ingroup format
         */
        class AircraftFileFormat {
        private:
            std::map<std::string, types::Aircraft> m_aircrafts;

            void parseAircraft(const std::string& line);

        public:
            /**
             * @brief Parses an aircraft definition
             * If the file does not exist, the constructor throws an exeption.
             * @param[in] filename The filename of the configuration
             */
            AircraftFileFormat(const std::string& filename);

            /**
             * @brief Returns the map of aircrafts
             * @return The aircrafts
             */
            const std::map<std::string, types::Aircraft>& aircrafts() const;
        };
    }
}
