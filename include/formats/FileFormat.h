/*
 * @brief Defines the base class for all file formats
 * @file formats/FileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <map>

#include <types/Runway.h>
#include <types/Sector.h>

namespace topskytower {
    namespace formats {
        /**
         * @brief Defines the base class for all file formats
         * @ingroup format
         *
         * The base class provides error information, etc.
         */
        class FileFormat {
        protected:
            std::uint32_t m_errorLine;
            std::string   m_errorMessage;

            /**
             * @brief Creates the default file format
             */
            FileFormat();

            /**
             * @brief Resets the internal structures
             */
            void reset();

        public:
            /**
             * @brief Checks if an error was found
             * @return True if an error was found, else false
             */
            bool errorFound() const;
            /**
             * @brief Returns the error line
             * @return The error line
             */
            std::uint32_t errorLine() const;
            /**
             * @brief Returns the error message
             * @return The error message
             */
            const std::string& errorMessage() const;
        };
    }
}
