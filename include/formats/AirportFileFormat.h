/*
 * @brief Defines the airport file format
 * @file configuration/AirportFileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <map>

#include <formats/EseFileFormat.h>
#include <types/AirportConfiguration.h>

namespace topskytower {
    namespace formats {
        /**
         * @brief Defines the parser of airport configurations
         * @ingroup format
         */
        class AirportFileFormat {
        private:
            std::map<std::string, types::AirportConfiguration> m_configurations;

            bool parseSid(const std::string& line, types::StandardInstrumentDeparture& sid);

        public:
            /**
             * @brief Parses an airport configuration
             * If the file does not exist, the constructor throws an exeption.
             * @param[in] filename The filename of the configuration
             */
            AirportFileFormat(const std::string& filename);

            /**
             * @brief Returns the airport configuration of a special ICAO
             * @param[in] icao The airport's ICAO code
             * @return The airport configuration
             */
            const types::AirportConfiguration& configuration(const std::string& icao) const;
        };
    }
}
