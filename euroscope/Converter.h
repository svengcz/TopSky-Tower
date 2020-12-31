/*
 * @brief Defines the EuroScope to TopSky-Tower converter
 * @file euroscope/Converter.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#include <string>

#pragma warning(push, 0)
#include <EuroScopePlugIn.h>
#pragma warning(pop)

#include <types/ControllerInfo.h>
#include <types/Flight.h>

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the different ES converters
         * @ingroup euroscope
         */
        class Converter {
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
             * @param[in] airport The controlled airport to estimate departures and arrivals
             * @return The converted flight structure
             */
            static types::Flight convert(const EuroScopePlugIn::CRadarTarget& target, const std::string& airport);
            /**
             * @brief Converts an ES controller structure into a TopSky-Tower controller information
             * @param[in] controller The ES controller
             * @return The converted controller information
             */
            static types::ControllerInfo convert(const EuroScopePlugIn::CController& controller);
        };
    }
}
