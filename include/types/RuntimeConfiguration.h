/*
 * @brief Defines a runtime configuration
 * @file types/RuntimeConfiguration.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <list>
#include <map>
#include <string>

#include <types/Quantity.hpp>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a runtime configuration
         * @ingroup types
         */
        struct RuntimeConfiguration {
            bool ariwsActive;                                                     /**< Marks if ARIWS is active */
            bool cmacActive;                                                      /**< Marks if CMAC is active */
            bool lowVisibilityProcedures;                                         /**< Marks if LVP are active */
            std::map<std::string, std::list<std::string>> activeDepartureRunways; /**< Defines all active departure runways per active airport */
            std::map<std::string, std::list<std::string>> activeArrivalRunways;   /**< Defines all active arrival runways per active airport */

            RuntimeConfiguration() :
                    ariwsActive(true),
                    cmacActive(true),
                    lowVisibilityProcedures(false),
                    activeDepartureRunways(),
                    activeArrivalRunways() { }
        };
    }
}
