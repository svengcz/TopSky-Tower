/*
 * @brief Defines the EuroScope plug-in
 * @file euroscope/PlugIn.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#pragma warning(push, 0)
#include <EuroScopePlugIn.h>
#pragma warning(pop)

#include "RadarScreen.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the EuroScope plug-in
         * @ingroup euroscope
         */
        class PlugIn : public EuroScopePlugIn::CPlugIn {
        private:
            enum class TagItemElement {
                HandoffFrequency = 2001,
                ManuallyAlerts0  = 2002,
                ManuallyAlerts1  = 2003,
                ManuallyAlerts2  = 2004
            };
            enum class TagItemFunction {
                AircraftControlMenuBar   = 3000,
                AircraftControlSignal    = 3001,
                HandoffPerform           = 3002,
                HandoffControllerSelect  = 3003,
                HandoffSectorChangeEvent = 3004,
                HandoffSectorChange      = 3005,
                HandoffSectorSelect      = 3006
            };

            std::list<RadarScreen*> m_screens;

            void handleHandoffPerform(RECT area, const std::string& callsign, bool release, bool tracked);
            static bool visualizeManuallyAlerts(const types::Flight& flight, int idx, char itemString[16]);
            static void updateManuallyAlerts(EuroScopePlugIn::CRadarTarget& target, const std::string& marker);
            static void updateFlightStrip(EuroScopePlugIn::CRadarTarget& target, int idx, const std::string& marker);

        public:
            /**
             * @brief Creates a new plug-in
             */
            PlugIn();
            /**
             * @brief Destroys all internal strcutures
             */
            ~PlugIn();

            /**
             * @brief Called as soon as a new RADAR screen needs to be created
             * @param[in] displayName The display's name
             * @param[in] needsRadarContent True of the screen needs RADAR content
             * @param[in] geoReferenced True if the positions are geo referenced
             * @param[in] canBeSaved True if the configurations can be saved
             * @param[in] canBeCreated True if the configuration can be created
             * @return The created RADAR screen
             */
            EuroScopePlugIn::CRadarScreen* OnRadarScreenCreated(const char* displayName, bool needsRadarContent, bool geoReferenced,
                                                                bool canBeSaved, bool canBeCreated) override;
            /**
             * @brief Gets called as soon as the active runways changed
             */
            void OnAirportRunwayActivityChanged() override;
            /**
             * @brief Called as soon as the controller sent a commandline command
             * @param[in] cmdline The command
             * @return True if the command was handled, else false
             */
            bool OnCompileCommand(const char* cmdline) override;
            /**
             * @brief Called as soon as a tag entry needs to be updated
             * @param[in] flightPlan The requested flight plan
             * @param[in] radarTarget The corresponding RADAR target
             * @param[in] itemCode The requested item
             * @param[in] tagData The requested tag data
             * @param[in] itemString The buffer to store the data
             * @param[in] colorCode The color code of the entry
             * @param[in] rgb The colorization of the entry
             * @param[in] fontSize The size of the text
             */
            void OnGetTagItem(EuroScopePlugIn::CFlightPlan flightPlan, EuroScopePlugIn::CRadarTarget radarTarget,
                              int itemCode, int tagData, char itemString[16], int* colorCode, COLORREF* rgb,
                              double* fontSize) override;
            /**
             * @brief Called as soon as a function is triggered
             * @param[in] functionId The triggered ID
             * @param[in] itemString The content of the message
             * @param[in] pt The click position
             * @param[in] area The clicked area
             */
            void OnFunctionCall(int functionId, const char* itemString, POINT pt, RECT area) override;
        };
    }
}
