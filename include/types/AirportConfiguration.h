/*
 * @brief Defines the structure of the airport configuration
 * @file types/AirportConfiguration.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include <types/Aircraft.h>
#include <types/Coordinate.h>
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
         * @brief Defines constraints for specific destinations
         */
        struct DestinationConstraint {
            std::string   destination;        /**< The destination's ICAO */
            bool          evenCruiseLevel;    /**< Marks if an even flight level is required */
            types::Length minimumCruiseLevel; /**< Defines the minimum flight level */
            types::Length maximumCruiseLevel; /**< Defines the maximum flight level */
        };

        /**
         * @brief Defines a stand with all relevant information
         */
        struct Stand {
            std::string                            name;                /**< The stand's name */
            types::Coordinate                      position;            /**< The stand's coordinate */
            types::Length                          assignmentRadius;    /**< The stand's automatic assignment radius */
            int                                    priority;            /**< The generic stand priority during the assignment */
            std::vector<std::string>               blockingStands;      /**< All stands that are blocked if this stand is occupied */
            bool                                   manualAssignment;    /**< This stand is ignored by the automatic assignment */
            types::Length                          wingspan[2];         /**< The allowed aircraft wingspan range */
            types::Length                          length[2];           /**< The allowed aircraft length range */
            types::Length                          height[2];           /**< The allowed aircraft height range */
            std::list<types::Aircraft::WTC>        wtcWhitelist;        /**< All allowed WTC categories */
            std::list<types::Aircraft::WTC>        wtcBlacklist;        /**< All non-allowed WTC categories */
            std::list<types::Aircraft::EngineType> engineTypeWhitelist; /**< All allowed engine types */
            std::list<types::Aircraft::EngineType> engineTypeBlacklist; /**< All non-allowed engine types */
        };

        /**
         * @brief Describes a airport configuration
         * @ingroup types
         */
        struct AirportConfiguration {
            bool                                               valid;                  /**< Marks if the configuration is valid */
            std::map<std::string, StandardInstrumentDeparture> sids;                   /**< The SIDs per airports */
            std::list<DestinationConstraint>                   destinationConstraints; /**< The constraints of specific destinations */
            std::list<Stand>                                   aircraftStands;         /**< The list of all available aircraft stands */

            /**
             * @brief Creates an empty and unintialized airport configuration
             */
            AirportConfiguration() :
                    valid(false),
                    sids(),
                    destinationConstraints(),
                    aircraftStands() { }
        };
    }
}
