/*
 * @brief Defines a system configuration
 * @file types/SystemConfiguration.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a controller information
         * @ingroup types
         */
        struct SystemConfiguration {
            bool valid;                        /**< Marks if the configuration is valid */
            std::string hoppiesCode;           /**< Defines Hoppie's logon-code */
            std::uint8_t uiBackgroundColor[3]; /**< Defines the background color of the UI elements */
            std::uint8_t uiForegroundColor[3]; /**< Defines the foreground color of the UI elements */

            /**
             * @brief Creates an empty and uninitialized system configuration
             */
            SystemConfiguration() :
                    valid(false),
                    hoppiesCode(),
                    uiBackgroundColor{ 0, 0, 0 },
                    uiForegroundColor{ 0, 0, 0, } { }
        };
    }
}