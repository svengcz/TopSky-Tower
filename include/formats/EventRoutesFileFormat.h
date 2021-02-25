/*
 * @brief Defines the event routes file format
 * @file formats/EventRoutesFileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <formats/FileFormat.h>
#include <helper/String.h>
#include <types/EventRoutesConfiguration.h>

namespace topskytower {
    namespace formats {
        /**
         * @brief Defines the event routes file format
         * @ingroup format
         */
        class EventRoutesFileFormat : public FileFormat {
        private:
#ifndef DOXYGEN_IGNORE
            std::string m_filename;

            bool mergeEvents(std::map<std::string, types::Event>& events, types::Event& event, std::size_t lineCount);

        public:
            /**
             * @brief Parses an event routes configuration
             * If the file does not exist, the constructor throws an exeption.
             * @param[in] filename The filename of the configuration
             */
            EventRoutesFileFormat(const std::string& filename);

            /**
             * @brief Parses the set configuration file
             * @param[out] config The resulting configuration
             * @return True if the configuration file was valid, else false
             */
            bool parse(types::EventRoutesConfiguration& config);
#endif
        };
    }
}
