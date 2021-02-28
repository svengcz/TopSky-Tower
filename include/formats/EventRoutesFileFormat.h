/*
 * @brief Defines the event routes file format
 * @file formats/EventRoutesFileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <formats/FileFormat.h>
#include <helper/String.h>
#include <types/EventRoutesConfiguration.h>

namespace topskytower {
    namespace formats {
        /**
         * @brief Defines the event routes file format
         * @ingroup format
         *
         * The event routes file format describes the way to describe VATSIM-event specific routes.
         * It allows to define valid routes that need to be flown by aircrafts that want to participate in the events.
         *
         * An event can have multiple entries to specifiy different routes and it allows different flight level rules
         * and minimum and maximum flight levels.
         *
         * The event definition has to begin with the following block:
         * @code{.xml}
         * EVENT:NAME
         * AIRPORTS:ORIGIN:DESTINATION
         * @endcode
         *
         * The name will be used in the window to activate and deactivate the event routes.
         * It is required to define the origin and destination of the event city pair.
         * If an event includes multiple airports is it required to define multiple events with the same name.
         * All events with the same name will be clustered to one event for the controller.
         *
         * Afterwards is it possible to describe flight level constraints and rules that are required for flightplans.
         * @code{.xml}
         * LEVELS:MIN:MAX:[RULE]
         * @endcode
         *
         * The minimum and maximum flight level are described by three digits and the flight rule has to contain
         * an 'E' for even flight levels or 'O' for odd ones. The complete LEVELS-block is optional and the default
         * configuration will be FL000 to FL999 with all kinds of allowed flight levels.
         *
         * Afterwards are the route definitions which take the city-pair and flight level definitions.
         * @code{.xml}
         * ROUTE:WAYPOINTS AND AIRWAYS
         * @endcode
         *
         * The first waypoint needs to be the exit of the SID and the last waypoint needs to be the entry of the STAR or transition.
         * It is possible to define multiple routes for one event. It takes the preceeding city-pair and flight level constraints.
         * The route needs to be as short as possible. Waypoints on the same airway need to be eliminated and DCTs need to be removed.
         * An example below shows how a route can look like. The first line shows a normal filed route and the second one that is used by TopSky-Tower.
         * The second line needs to be set in the configuration.
         * @code{.xml}
         * ROUTE:POVEL DCT ALOSI T157 KERAX
         * ROUTE:POVEL ALOSI T157 KERAX
         * @endcode
         *
         * TopSky-Tower optimizes the filed route of a pilot in that way that it contains only the relevant and minimal information.
         * It erases waypoints for step-climbs, the step-climbs and DCT entries out of the route to compare it with the minimal route of the configuration.
         *
         * If the flight level constraints change between different routes is it possible to define LEVELS/ROUTE pairs
         * before the new route definition.
         */
        class EventRoutesFileFormat : public FileFormat {
        private:
#ifndef DOXYGEN_IGNORE
            std::string m_filename;

            bool mergeEvents(std::map<std::string, types::Event>& events, types::Event& event, std::size_t lineCount);

        public:
            /**
             * @brief Parses an event routes configuration
             * If the file does not exist, the constructor throws an exeption.
             * @param[in] filename The filename of the configuration
             */
            EventRoutesFileFormat(const std::string& filename);

            /**
             * @brief Parses the set configuration file
             * @param[out] config The resulting configuration
             * @return True if the configuration file was valid, else false
             */
            bool parse(types::EventRoutesConfiguration& config);
#endif
        };
    }
}
