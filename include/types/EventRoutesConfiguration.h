/*
 * @brief Defines a runtime configuration
 * @file types/RuntimeConfiguration.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <list>
#include <string>

#include <types/Quantity.hpp>

namespace topskytower {
    namespace types {
        /**
         * @brief Defines a single route of an event
         * @ingroup types
         */
        struct EventRoute {
            enum class EvenOddRule {
                Undefined = 0,
                Even      = 1,
                Odd       = 2
            };

            std::string   origin;       /**< The origin airport */
            std::string   destination;  /**< The destination airport */
            std::string   route;        /**< The complete route */
            types::Length minimumLevel; /**< The minimum flight level */
            types::Length maximumLevel; /**< The maximum flight level */
            EvenOddRule   rule;         /**< The flight level rule */

            /**
             * @brief Creates an empty route
             */
            EventRoute() :
                    origin(),
                    destination(),
                    route(),
                    minimumLevel(),
                    maximumLevel(99000_ft),
                    rule(EvenOddRule::Undefined) { }
        };

        /**
         * @brief Defines an event
         * @ingroup types
         */
        struct Event {
            std::string           name;   /**< The event's name */
            bool                  active; /**< Marks if the event is active */
            std::list<EventRoute> routes; /**< The event routes */

            /**
             * @brief Creates an empty event
             */
            Event() :
                    name(),
                    active(false),
                    routes() { }
        };

        /**
         * @brief Describes the event routes to validate them in the flight plan checker
         * @ingroup types
         */
        struct EventRoutesConfiguration {
            bool             valid;  /**< Defines if the configuration is valid */
            std::list<Event> events; /**< Defines the events */

            /**
             * @brief Creates an empty configuration
             */
            EventRoutesConfiguration() :
                    valid(false),
                    events() { }
        };
    }
}
