/*
 * @brief Defines the airport file format
 * @file configuration/AirportFileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

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
            types::AirportConfiguration m_configuration;

            static bool parseSid(const std::vector<std::string>& elements, types::StandardInstrumentDeparture& sid);
            static bool parseConstraint(const std::vector<std::string>& elements, types::DestinationConstraint& constraint);
            bool parseDepartures(const std::vector<std::string>& lines);
            static bool parseStandDefinition(const std::vector<std::string>& elements, types::Stand& stand);
            static bool parseWingspan(const std::vector<std::string>& elements, types::Stand& stand);
            static bool parseLength(const std::vector<std::string>& elements, types::Stand& stand);
            static bool parseHeight(const std::vector<std::string>& elements, types::Stand& stand);
            static bool parseWtc(const std::string& categories, std::list<types::Aircraft::WTC>& list);
            static bool parseEngineType(const std::string& types, std::list<types::Aircraft::EngineType>& list);
            bool parseStands(const std::vector<std::string>& lines);

        public:
            /**
             * @brief Parses an airport configuration
             * If the file does not exist, the constructor throws an exeption.
             * @param[in] filename The filename of the configuration
             */
            AirportFileFormat(const std::string& filename);

            /**
             * @brief Returns the airport configuration
             * @return The airport configuration
             */
            const types::AirportConfiguration& configuration() const;
        };
    }
}
