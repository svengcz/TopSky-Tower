/*
 * @brief Defines a runtime configuration
 * @file types/RuntimeConfiguration.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>

#include <types/Quantity.hpp>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a runtime configuration
         * @ingroup types
         */
        struct RuntimeConfiguration {
            bool ariwsActive;             /**< Marks if ARIWS is active */
            bool cmacActive;              /**< Marks if CMAC is active */
            bool lowVisibilityProcedures; /**< Marks if LVP are active */

            RuntimeConfiguration() :
                    ariwsActive(true),
                    cmacActive(true),
                    lowVisibilityProcedures(false) { }
        };
    }
}
