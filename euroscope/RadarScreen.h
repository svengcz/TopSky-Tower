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

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the RADAR screen
         * @ingroup euroscope
         */
        class RadarScreen : public EuroScopePlugIn::CRadarScreen {
        private:
            bool                          m_initialized;
            std::string                   m_airport;
            surveillance::SectorControl*  m_controllers;
            surveillance::FlightRegistry* m_flights;
            std::mutex                    m_disconnectedFlightsLock;
            std::list<std::string>        m_disconnectedFlights;

            void initialize();

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
             * @brief Returns the constant flight registry
             * @return The flight registry
             */
            const surveillance::FlightRegistry& flightRegistry() const;
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
        };
    }
}
