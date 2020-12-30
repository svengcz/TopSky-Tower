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
            bool valid;              /**< Marks if the configuration is valid */
            std::string hoppiesCode; /**< Defines Hoppie's logon-code */

            /**
             * @brief Creates an empty and uninitialized system configuration
             */
            SystemConfiguration() :
                    valid(false),
                    hoppiesCode() { }
        };
    }
}
