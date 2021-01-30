/*
 * @brief Defines the EuroScope to TopSky-Tower converter
 * @file euroscope/Converter.h
 * @author Sven Czarnian <devel@svcz.de>
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
            static std::string findScratchPadEntry(const EuroScopePlugIn::CFlightPlan& plan, const std::string& marker, const std::string& entry);
            static bool convertAtcCommand(const EuroScopePlugIn::CFlightPlan& plan, types::FlightPlan& flightPlan);
            static void convertStandAssignment(const EuroScopePlugIn::CFlightPlan& plan, const types::Flight& flight, RadarScreen& screen);

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
             * @param[out] overwriteStatus Marks if we have to overwrite the ground status without heuristics
             * @return The converted flight plan
             */
            static types::FlightPlan convert(const EuroScopePlugIn::CFlightPlan& plan, bool& overwriteStatus);
            /**
             * @brief Converts an ES radar target into a TopSky-Tower flight structure
             * @param[in] target The radar target
             * @param[in] screen The corresponding RADAR screen
             * @param[out] overwriteStatus Marks if we have to overwrite the ground status without heuristics
             * @return The converted flight structure
             */
            static types::Flight convert(const EuroScopePlugIn::CRadarTarget& target, RadarScreen& screen, bool& overwriteStatus);
            /**
             * @brief Converts an ES controller structure into a TopSky-Tower controller information
             * @param[in] controller The ES controller
             * @return The converted controller information
             */
            static types::ControllerInfo convert(const EuroScopePlugIn::CController& controller);
        };
    }
}
