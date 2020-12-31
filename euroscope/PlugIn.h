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

#include <types/AirportConfiguration.h>
#include <types/SystemConfiguration.h>

#include "RadarScreen.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the EuroScope plug-in
         * @ingroup euroscope
         */
        class PlugIn : public EuroScopePlugIn::CPlugIn {
        public:
            /**
             * @brief Defines the different internal and external tag functions
             */
            enum class TagItemFunction {
                AircraftControlMenuBar              = 3000, /**< The generic aircraft menu */
                AircraftControlSignal               = 3001, /**< One marker or flag of the flight is set */
                HandoffPerform                      = 3002, /**< Perform the handoff to the next controller */
                HandoffControllerSelectEvent        = 3003, /**< Select the next controller event */
                HandoffControllerSelect             = 3004, /**< Select the next controller */
                HandoffSectorChangeEvent            = 3005, /**< Select the next sector of a flight event */
                HandoffSectorChange                 = 3006, /**< Select the next sector of a flight */
                HandoffSectorSelect                 = 3007, /**< Overwrite the current selected sector */
                SectorControllerHandover            = 3008, /**< A handover of the own sector is ongoing */
                SectorControllerHandoverSelectEvent = 3009, /**< A handover of the own sector is ongoing */
                SectorControllerHandoverSelect      = 3010, /**< A handover of the own sector is ongoing */
                PdcMenu                             = 3011, /**< Open the PDC menu */
                PdcReadMessage                      = 3012, /**< Read the next PDC message */
                UiElementIds                        = 4000, /**< Elements of the UI entries */
                UiEditTextRequest                   = 4001, /**< Request an edit-field */
                UiEditTextResponse                  = 4002  /**< Get the answer of the requested edit-field */
            };

        private:
            enum class TagItemElement {
                HandoffFrequency = 2001,
                ManuallyAlerts0  = 2002,
                ManuallyAlerts1  = 2003,
                ManuallyAlerts2  = 2004,
                FlightMarker     = 2005,
                PdcIndicator     = 2006
            };

            std::string                             m_settingsPath;
            std::list<RadarScreen*>                 m_screens;
            std::function<void(const std::string&)> m_uiCallback;

            void handleHandoffPerform(POINT point, RECT area, const std::string& callsign, bool tracked);
            static bool visualizeManuallyAlerts(const types::Flight& flight, int idx, char itemString[16]);
            static void updateManuallyAlerts(EuroScopePlugIn::CRadarTarget& target, const std::string& marker);
            static void updateFlightStrip(EuroScopePlugIn::CRadarTarget& target, int idx, const std::string& marker);
            RadarScreen* findLastActiveScreen();

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
            /**
             * @brief Returns the path of the settings files
             * @return The path to the settings files
             */
            const std::string& settingsPath() const;
            /**
             * @brief Registers a callback for an UI element
             * @tparam T The element which registers the callback
             * @tparam F The callback function
             * @param[in] instance The instance which registers the callback
             * @param[in] cbFunction The callback function
             */
            template <typename T, typename F>
            void registerUiCallback(T* instance, F cbFunction) {
                std::function<void(const std::string&)> func = std::bind(cbFunction, instance, std::placeholders::_1);
                this->m_uiCallback = func;
            }
        };
    }
}
