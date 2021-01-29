/*
 * @brief Defines the exception interface
 * @file helper/Exception.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <string>

namespace topskytower {
    namespace helper {
        /**
         * @brief Defines the exception which is used to describe internal errors, that need to be handled
         * @ingroup helper
         */
        class Exception {
        private:
            std::string m_sender;
            std::string m_message;

        public:
            /**
             * @brief Creates a new exception
             * @param[in] sender The sender of the exception
             * @param[in] message The error description which leads to this error
             */
            Exception(const std::string& sender, const std::string& message);

            /**
             * @brief Returns the sender of the exception
             * @return The constant reference to the sender's name
             */
            const std::string& sender() const;
            /**
             * @brief Returns the error message of the exception
             * @return The constant reference to the sender's error message
             */
            const std::string& message() const;
        };
    }
}
