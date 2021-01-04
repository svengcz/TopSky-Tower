/*
 * @brief Defines the structure of the airport configuration
 * @file types/AirportConfiguration.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <map>
#include <string>

#include <types/Aircraft.h>
#include <types/Quantity.hpp>

namespace topskytower {
    namespace types {
        /**
         * @brief Defines the SID with all given restrictions
         * @ingroup types
         */
        struct StandardInstrumentDeparture {
            std::string          name;                /**< SID's name */
            types::Length        clearanceLimit;      /**< SID's clearance limit */
            bool                 containsStepClimbs;  /**< SID contains step climbs or speed restrictions */
            Aircraft::EngineType engineType;          /**< SID requires special engine type */
            bool                 requiresTransponder; /**< SID requires a A+C transponder */
            bool                 requiresRnav;        /**< SID requires RNAV-capabilities */
            types::Length        minimumCruiseLevel;  /**< SID requires a minimum flight level */
            types::Length        maximumCruiseLevel;  /**< SID requires a maximum flight level */
        };

        /**
         * @brief Describes a airport configuration
         * @ingroup types
         */
        struct AirportConfiguration {
            bool                                               valid; /**< Marks if the configuration is valid */
            std::map<std::string, StandardInstrumentDeparture> sids;  /**< The SIDs per airports */

            /**
             * @brief Creates an empty and unintialized airport configuration
             */
            AirportConfiguration() :
                    valid(false) { }
        };
    }
}
