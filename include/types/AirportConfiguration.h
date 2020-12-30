/*
 * @brief Defines the structure of the airport configuration
 * @file types/AirportConfiguration.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <map>
#include <string>

namespace topskytower {
    namespace types {
        /**
         * @brief Defines the SID
         * @ingroup types
         */
        struct StandardInstrumentDeparture {
            std::string   name;           /**< SID's name */
            std::uint16_t clearanceLimit; /**< SID's clearance limit */
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
