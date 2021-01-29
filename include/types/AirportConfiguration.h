/*
 * @brief Defines the structure of the airport configuration
 * @file types/AirportConfiguration.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
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
         * @brief Defines the stands for a specific priority
         */
        struct StandPriorities {
            int                    priority; /**< Defines the priority */
            std::list<std::string> stands;   /**< Defines the stands for this priority */
        };

        /**
         * @brief Defines all assigned stands that are used by an airline
         */
        struct AirlineStandAssignments {
            std::string                airlineIcao;     /**< The airline's ICAO code */
            std::list<StandPriorities> standPriorities; /**< All stand priorities */
        };

        /**
         * @brief Defines a holding point to enter a runway
         */
        struct HoldingPoint {
            std::string       runway;        /**< Defines the active runway for this holding point */
            bool              lowVisibility; /**< Defines if the holding point is used during low visibility */
            types::Coordinate holdingPoint;  /**< Defines the center position of the holding point */
            types::Angle      heading;       /**< Defines the heading to the runway */
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
            std::list<AirlineStandAssignments>                 airlines;               /**< The airline to stand assignments */
            std::list<HoldingPoint>                            holdingPoints;          /**< The holding points of the airport */
            std::map<std::string, std::list<std::string>>      ipaRunways;             /**< Defines the possible IPA combinations */
            std::map<std::string, std::list<std::string>>      prmRunways;             /**< Defines the possible PRM combinations */

            /**
             * @brief Creates an empty and unintialized airport configuration
             */
            AirportConfiguration() :
                    valid(false),
                    sids(),
                    destinationConstraints(),
                    aircraftStands(),
                    airlines(),
                    ipaRunways(),
                    prmRunways() { }
        };
    }
}
