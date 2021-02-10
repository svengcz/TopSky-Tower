/*
 * @brief Defines the EuroScope to TopSky-Tower converter
 * @file euroscope/Converter.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <string>

#pragma warning(push, 0)
#include <EuroScopePlugIn.h>
#pragma warning(pop)

#include <types/ControllerInfo.h>
#include <types/Flight.h>

namespace topskytower {
    namespace euroscope {
        class RadarScreen;

        /**
         * @brief Defines the different ES converters
         * @ingroup euroscope
         */
        class Converter {
        private:
            static void convertAtcCommand(const EuroScopePlugIn::CFlightPlan& plan, types::FlightPlan& flightPlan);

        public:
            /**
             * @brief Converts an ES position to a TopSky-Tower coordinate
             * @param[in] position The ES position
             * @return The converted coordinate
             */
            static types::Coordinate convert(const EuroScopePlugIn::CPosition& position);
            /**
             * @brief Converts an ES flight plan into a TopSky-Tower flight structure
             * @param[in] plan The flight plan
             * @return The converted flight plan
             */
            static types::FlightPlan convert(const EuroScopePlugIn::CFlightPlan& plan);
            /**
             * @brief Converts an ES radar target into a TopSky-Tower flight structure
             * @param[in] target The radar target
             * @return The converted flight structure
             */
            static types::Flight convert(const EuroScopePlugIn::CRadarTarget& target);
            /**
             * @brief Converts an ES controller structure into a TopSky-Tower controller information
             * @param[in] controller The ES controller
             * @return The converted controller information
             */
            static types::ControllerInfo convert(const EuroScopePlugIn::CController& controller);
        };
    }
}
