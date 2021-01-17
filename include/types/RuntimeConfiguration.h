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
         * @brief Defines the weather data
         */
        struct WeatherData {
            WindData      wind; /**< Defines the wind data */
            std::uint16_t qnh;  /**< Defines the current QNH */
        };

        /**
         * @brief Describes a runtime configuration
         * @ingroup types
         */
        struct RuntimeConfiguration {
            bool ariwsActive;                                                     /**< Marks if ARIWS is active */
            bool cmacActive;                                                      /**< Marks if CMAC is active */
            bool mtcdActive;                                                      /**< Marks if MTCD is active */
            bool lowVisibilityProcedures;                                         /**< Marks if LVP are active */
            std::map<std::string, std::list<std::string>> activeDepartureRunways; /**< Defines all active departure runways per active airport */
            std::map<std::string, std::list<std::string>> activeArrivalRunways;   /**< Defines all active arrival runways per active airport */
            std::map<std::string, WeatherData> weatherInformation                 /**< Defines the weather on the airport */

            RuntimeConfiguration() :
                    ariwsActive(true),
                    cmacActive(true),
                    mtcdActive(true),
                    lowVisibilityProcedures(false),
                    activeDepartureRunways(),
                    activeArrivalRunways(),
                    windInformation() { }
        };
    }
}
