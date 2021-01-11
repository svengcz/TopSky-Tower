/*
 * @brief Defines the EuroScope RADAR screen
 * @file euroscope/RadarScreen.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <mutex>
#include <string>

#pragma warning(push, 0)
#include <EuroScopePlugIn.h>
#pragma warning(pop)

#include <surveillance/ARIWSControl.h>
#include <surveillance/SectorControl.h>
#include <surveillance/StandControl.h>
#include <system/FlightRegistry.h>

#include "ui/UiManager.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the RADAR screen
         * @ingroup euroscope
         */
        class RadarScreen : public EuroScopePlugIn::CRadarScreen {
        public:
            /**
             * @brief Defines the different click elements
             */
            enum class ClickId {
                UserWindow = 40 /**< A user windows is clicked */
            };

            /**
             * @brief Defines an ES-event that is called during the OnRefresh-call
             */
            struct EuroscopeEvent {
                int         tagItemFunction; /**< The ID of the function */
                std::string callsign;        /**< The flights callsign */
                std::string itemString;      /**< The item string */
                POINT       point;           /**< The position of the event */
                RECT        area;            /**< The area of the event */
            };

        private:
            bool                                  m_initialized;
            std::string                           m_airport;
            UiManager                             m_userInterface;
            system::FlightRegistry*               m_flightRegistry;
            surveillance::SectorControl*          m_sectorControl;
            surveillance::StandControl*           m_standControl;
            surveillance::ARIWSControl*           m_ariwsControl;
            std::mutex                            m_guiEuroscopeEventsLock;
            std::list<EuroscopeEvent>             m_guiEuroscopeEvents;
            std::chrono::system_clock::time_point m_lastRenderingTime;

            void initialize();
            std::vector<types::Coordinate> extractPredictedSID(const std::string& callsign, const types::Coordinate& sidExit);

        public:
            /**
             * @brief Creates a new RADAR screen
             */
            RadarScreen();
            /**
             * @brief Destroys all internal structures
             */
            ~RadarScreen();

            /**
             * @brief Called as soon as an ASR file is loaded
             * @param[in] loaded True if the file is loaded
             */
            void OnAsrContentLoaded(bool loaded) override;
            /**
             * @brief Called as soon as the ASR file is closed
             */
            void OnAsrContentToBeClosed() override;
            /**
             * @brief Called as soon as the controller clicks on the screen
             * @param[in] objectType The type of the click
             * @param[in] objectId The ID of the clicked object
             * @param[in] pt The click position
             * @param[in] area The clicked area
             * @param[in] button The ID of the mouse button
             */
            void OnClickScreenObject(int objectType, const char* objectId, POINT pt, RECT area, int button) override;
            /**
             * @brief Called as soon as the controller clicks on the screen and keeps the button pressed
             * @param[in] objectType The type of the click
             * @param[in] objectId The ID of the clicked object
             * @param[in] pt The click position
             * @param[in] area The clicked area
             * @param[in] released Marks if the mouse button is released or not
             */
            void OnMoveScreenObject(int objectType, const char* objectId, POINT pt, RECT area, bool released) override;
            /**
             * @brief Called as soon as a controller station is updated
             * @param[in] controller The updated controller station
             */
            void OnControllerPositionUpdate(EuroScopePlugIn::CController controller) override;
            /**
             * @brief Called as soon as a controller station is offline
             * @param[in] controller The disconnected controller station
             */
            void OnControllerDisconnect(EuroScopePlugIn::CController controller) override;
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
             * @brief Called as soon as the screen needs to be rendered
             * @param[in] hdc The window instance
             * @param[in] phase The rendering phase
             */
            void OnRefresh(HDC hdc, int phase) override;
            /**
             * @brief Returns the changable controller manager
             * @return The controller manager
             */
            surveillance::SectorControl& sectorControl();
            /**
             * @brief Returns the flight registry
             * @return The flight registry
             */
            system::FlightRegistry& flightRegistry() const;
            /**
             * @brief Returns the stand control
             * @return The stand control
             */
            surveillance::StandControl& standControl() const;
            /**
             * @brief Returns the ARIWS control
             * @return The ARIWS control
             */
            surveillance::ARIWSControl& ariwsControl() const;
            /**
             * @brief Registers an Euroscope GUI event to trigger the function call during the next rendering step
             * @param[in] entry The new GUI event
             */
            void registerEuroscopeEvent(EuroscopeEvent&& entry);
            /**
             * @brief Returns the airport's ICAO
             * @return The airport's ICAO
             */
            const std::string& airportIcao() const;
            /**
             * @brief Returns the user-interface manager
             * @return Reference to the manager
             */
            UiManager& uiManager();
            /**
             * @brief Checks if the screen is initialized
             * @return True if all information are available, else false
             */
            bool isInitialized() const;
            /**
             * @brief Returns the timestamp of the last rendering
             * @return The last rendering time
             */
            const std::chrono::system_clock::time_point& lastRenderingTime() const;
        };
    }
}
