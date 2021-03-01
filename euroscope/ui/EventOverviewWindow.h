/*
 * @brief Defines the event overview window
 * @file ui/EventOverviewWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <mutex>

#include <system/ConfigurationRegistry.h>

#include "elements/TableViewer.h"
#include "InsetWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the inset window to give an overview about all event route checks
         * @ingroup euroscope
         */
        class EventOverviewWindow : public InsetWindow {
        private:
            bool         m_firstRendering;
            bool         m_updateViaClick;
            TableViewer* m_eventOverview;

            void setOverviewContent(system::ConfigurationRegistry::UpdateType type);

        public:
            /**
             * @brief Creates a new event overview viewer
             * @param[in] parent The corresponding RADAR screen
             */
            EventOverviewWindow(RadarScreen* parent);
            /**
             * @brief Destroys the window
             */
            ~EventOverviewWindow();

            /**
             * @brief Handles the click events
             * @param[in] pt The position of the mouse
             * @param[in] button The clicked mouse button
             * @return True if the click was handled, else false
             */
            bool click(const Gdiplus::PointF& pt, UiManager::MouseButton button) override;
            /**
             * @brief Visualizes the window
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            bool visualize(Gdiplus::Graphics* graphics) override;
        };
    }
}
