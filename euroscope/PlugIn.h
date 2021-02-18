/*
 * @brief Defines the EuroScope plug-in
 * @file euroscope/PlugIn.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <functional>

#pragma warning(push, 0)
#include <EuroScopePlugIn.h>
#pragma warning(pop)

#include <surveillance/FlightPlanControl.h>
#include <types/AirportConfiguration.h>
#include <types/SystemConfiguration.h>

#include "HiddenWindow.h"
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
             * @brief Defines the indices of the flight strip annotations
             */
            enum class AnnotationIndex {
                Unknown    = -1, /**< The annotation is unknown */
                Stand      = 6,  /**< Defines the stand annotation */
                Marker     = 7,  /**< Defines the marker annotation */
            };

            /**
             * @brief Defines the different internal and external tag functions
             */
            enum class TagItemFunction {
                AircraftControlMenuBar              = 3000, /**< The generic aircraft menu */
                AircraftControlSignal               = 3001, /**< One marker or flag of the flight is set */
                SurveillanceAlertVisualization      = 3002, /**< Triggers the current surveillance visualization */
                DepartureRouteDrawTimeBased         = 3003, /**< Triggers the visualization of the predicted route for a given time */
                DepartureRouteDraw                  = 3004, /**< Triggers the visualization of the predicted route */
                HandoffPerform                      = 3100, /**< Perform the handoff to the next controller */
                HandoffControllerSelectEvent        = 3101, /**< Select the next controller event */
                HandoffControllerSelect             = 3102, /**< Select the next controller */
                HandoffSectorChangeEvent            = 3103, /**< Select the next sector of a flight event */
                HandoffSectorChange                 = 3104, /**< Select the next sector of a flight */
                HandoffSectorSelect                 = 3105, /**< Overwrite the current selected sector */
                SectorControllerHandover            = 3200, /**< A handover of the own sector is ongoing */
                SectorControllerHandoverSelectEvent = 3201, /**< A handover of the own sector is ongoing */
                SectorControllerHandoverSelect      = 3202, /**< A handover of the own sector is ongoing */
                PdcMenu                             = 3300, /**< Open the PDC menu */
                PdcReadMessage                      = 3301, /**< Read the next PDC message */
                PdcSendStandby                      = 3302, /**< Send a stand-by message */
                PdcSendClearance                    = 3303, /**< Sends the departure clearance message */
                FlightPlanCheckMenu                 = 3400, /**< Opens the flight plan check menu */
                FlightPlanCheckErrorLog             = 3401, /**< Opens a window with all error logs */
                FlightPlanCheckOverwrite            = 3402, /**< Overwrites detected errors and marks the flight plan as OK */
                StandControlMenu                    = 3500, /**< Opens the stand control menu */
                StandControlPublish                 = 3501, /**< Publishs the current stand */
                StandControlAutomatic               = 3502, /**< Assigns a stand automatically to a flight */
                StandControlManualEvent             = 3503, /**< Triggers the stand selection event */
                StandControlManual                  = 3504, /**< Opens the selection menu */
                StandControlManualSelect            = 3505, /**< Selects the final stand */
                StandControlScreenSelect            = 3506, /**< Activates the stand selection on the screen */
                DepartureGroundStatusMenu           = 3600, /**< Opens the departure ground status menu */
                DepartureGroundStatusSelect         = 3601, /**< Sets the new status of the departure status */
                ArrivalGroundStatusMenu             = 3700, /**< Opens the arrival ground status menu */
                ArrivalGroundStatusSelect           = 3701, /**< Sets the new status of the arrival status */
                HoldingPointCandidatesMenu          = 3800, /**< Opens the holding point menu */
                HoldingPointCandidatesSelect        = 3801, /**< Sets the new used holding point */
                UiElementIds                        = 4000, /**< Elements of the UI entries */
                UiEditTextRequest                   = 4001, /**< Request an edit-field */
                UiEditTextResponse                  = 4002  /**< Get the answer of the requested edit-field */
            };

        private:
            enum class TagItemElement {
                HandoffFrequency      = 2001,
                ManuallyAlerts0       = 2002,
                ManuallyAlerts1       = 2003,
                ManuallyAlerts2       = 2004,
                FlightMarker          = 2005,
                PdcIndicator          = 2006,
                SIDStepClimbIndicator = 2007,
                FlighPlanCheck        = 2008,
                AircraftStand         = 2009,
                DepartureGroundStatus = 2010,
                ArrivalGroundStatus   = 2011,
                SurveillanceAlerts    = 2012,
                HoldingPoint          = 2013
            };

            bool                                    m_errorMode;
            std::string                             m_settingsPath;
            std::list<RadarScreen*>                 m_screens;
            std::function<void(const std::string&)> m_uiCallback;
            std::string                             m_pdcNotificationSound;
            WNDCLASSA                               m_windowClass;
            HWND                                    m_hiddenWindow;

            static std::string findScratchPadEntry(const EuroScopePlugIn::CFlightPlan& plan, const std::string& marker,
                                                   const std::string& entry);
            void handleHandoffPerform(POINT point, RECT area, const types::Flight& flight, bool tracked,
                                      RadarScreen* screen);
            static bool visualizeManuallyAlerts(const types::Flight& flight, int idx, char itemString[16]);
            static void updateManuallyAlerts(EuroScopePlugIn::CRadarTarget& target, const std::string& marker);
            RadarScreen* findLastActiveScreen();
            static bool summarizeFlightPlanCheck(const std::list<surveillance::FlightPlanControl::ErrorCode>& codes,
                                                 char* itemString, int* colorCode);
            static std::string flightPlanCheckResultLog(const std::list<surveillance::FlightPlanControl::ErrorCode>& codes);
            void pdcMessageReceived();
            void updateGroundStatus(EuroScopePlugIn::CRadarTarget target, const std::string_view& view,
                                    const types::Flight& flight, bool arrival);
            void updateStand(const types::Flight& flight, EuroScopePlugIn::CFlightPlan& plan);
            void updateHoldingPoint(const types::Flight& flight, EuroScopePlugIn::CFlightPlan& plan);

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
             * @brief Gets called as soon as the active runways changed
             */
            void OnAirportRunwayActivityChanged() override;
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
             * @brief Called as soon as metar information are updated
             * @param[in] station The updated station
             * @param[in] fullMetar The new metar
             */
            void OnNewMetarReceived(const char* station, const char* fullMetar) override;
            /**
             * @brief Called once per second
             * @param[in] counter The current call-cycle-counter
             */
            void OnTimer(int counter) override;
            /**
             * @brief Called as soon as a radar target position is updated
             * @param[in] radarTarget The updated radar target
             */
            void OnRadarTargetPositionUpdate(EuroScopePlugIn::CRadarTarget radarTarget) override;
            /**
             * @brief Called as soon as a flight plan is updated
             * @param[in] flightPlan The updated flight plan
             */
            void OnFlightPlanFlightPlanDataUpdate(EuroScopePlugIn::CFlightPlan flightPlan) override;
            /**
             * @brief Called as soon as a controller updated the flight plan
             * @param[in] flightPlan The updated flight plan
             * @param[in] type The changed information
             */
            void OnFlightPlanControllerAssignedDataUpdate(EuroScopePlugIn::CFlightPlan flightPlan, int type) override;
            /**
             * @brief Called as soon as a flight plan is offline
             * @param[in] flightPlan The disconnected flight plan
             */
            void OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan flightPlan) override;
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
            /**
             * @brief Removes the radar screen out of the registry
             * @param[in] screen The closed screen
             */
            void removeRadarScreen(RadarScreen* screen);
            /**
             * @brief Received a message from AfV
             * @param[in] message The incoming message
             */
            void afvMessage(const std::string& message);
        };
    }
}
