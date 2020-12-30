/*
 * @brief Defines the system file format
 * @file configuration/SystemConfiguration.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <types/SystemConfiguration.h>

namespace topskytower {
    namespace formats {
        /**
         * @brief Defines the parser of system configurations
         * @ingroup format
         */
        class SettingsFileFormat {
        private:
            std::string m_filename;

            static void parseColor(const std::string& block, std::uint8_t color[3]);

        public:
            /**
             * @brief Parses an airport configuration
             * If the file does not exist, the constructor throws an exeption.
             * @param[in] filename The filename of the configuration
             */
            SettingsFileFormat(const std::string& filename);

            /**
             * @brief Parses the set configuration file
             * @param[out] config The resulting configuration
             */
            void parse(types::SystemConfiguration& config) const;
        };
    }
}
