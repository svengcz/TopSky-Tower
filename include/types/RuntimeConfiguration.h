/*
 * @brief Defines a runtime configuration
 * @file types/RuntimeConfiguration.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <list>
#include <map>
#include <string>

#include <types/Quantity.hpp>

namespace topskytower {
    namespace types {
        /**
         * @brief Defines the wind data
         */
        struct WindData {
            bool            variable;  /**< Indicates if the wind direction is variable */
            types::Angle    direction; /**< Indicates the wind direction */
            types::Velocity speed;     /**< Indicates the wind speed */
            types::Velocity gusts;     /**< Indicates the wind gusts */

            /**
             * @brief Creates an empty wind-block
             */
            WindData() :
                    variable(false),
                    direction(),
                    speed(),
                    gusts() { }
        };

        /**
         * @brief Describes a runtime configuration
         * @ingroup types
         */
        struct RuntimeConfiguration {
            bool ariwsActive;                                                     /**< Marks if ARIWS is active */
            bool cmacActive;                                                      /**< Marks if CMAC is active */
            bool mtcdActive;                                                      /**< Marks if MTCD is active */
            bool stcdActive;                                                      /**< Marks if STCD is active */
            bool ipaActive;                                                       /**< Marks if IPA (independent parallel approach) is used */
            bool lowVisibilityProcedures;                                         /**< Marks if LVP are active */
            std::map<std::string, std::list<std::string>> activeDepartureRunways; /**< Defines all active departure runways per active airport */
            std::map<std::string, std::list<std::string>> activeArrivalRunways;   /**< Defines all active arrival runways per active airport */
            std::map<std::string, WindData> windInformation;                      /**< Defines the winds on the airport */

            RuntimeConfiguration() :
                    ariwsActive(true),
                    cmacActive(true),
                    mtcdActive(true),
                    stcdActive(true),
                    ipaActive(false),
                    lowVisibilityProcedures(false),
                    activeDepartureRunways(),
                    activeArrivalRunways(),
                    windInformation() { }
        };
    }
}
