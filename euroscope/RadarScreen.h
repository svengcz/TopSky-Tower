/*
 * @brief Defines the EuroScope RADAR screen
 * @file euroscope/RadarScreen.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#include <mutex>
#include <string>

#pragma warning(push, 0)
#include <EuroScopePlugIn.h>
#pragma warning(pop)

#include <surveillance/FlightRegistry.h>
#include <surveillance/SectorControl.h>

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
            bool                         m_updateFlightRegistry;
            bool                         m_initialized;
            std::string                  m_airport;
            UiManager                    m_userInterface;
            surveillance::SectorControl* m_controllers;
            std::mutex                   m_disconnectedFlightsLock;
            std::list<std::string>       m_disconnectedFlights;
            std::mutex                   m_guiEuroscopeEventsLock;
            std::list<EuroscopeEvent>    m_guiEuroscopeEvents;

            void initialize();

        public:
            /**
             * @brief Creates a new RADAR screen
             */
            RadarScreen();
            /**
             * @brief Creates a new RADAR screen
             * @param[in] updateFlightRegistry Defines if this screen is used to update the flight registry
             */
            RadarScreen(bool updateFlightRegistry);
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
             * @brief Returns the constant controller manager
             * @return The controller manager
             */
            const surveillance::SectorControl& sectorControl() const;
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
        };
    }
}
