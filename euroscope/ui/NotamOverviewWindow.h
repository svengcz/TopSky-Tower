/*
 * @brief Defines the NOTAM overview window
 * @file ui/NotamOverviewWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <management/NotamControl.h>

#include "elements/Checkbox.h"
#include "elements/DropDownMenu.h"
#include "elements/EditText.h"
#include "elements/TableViewer.h"
#include "InsetWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the inset window to give an overview NOTAM
         * @ingroup euroscope
         */
        class NotamOverviewWindow : public InsetWindow {
        private:
            bool          m_firstRendering;
            EditText*     m_airportFilter;
            DropDownMenu* m_categoryFilter;
            Checkbox*     m_activeFilter;
            TableViewer*  m_notamOverview;

            void setOverviewContent();
            static std::string translateCategory(management::NotamCategory category);

        public:
            /**
             * @brief Creates a new NOTAM overview viewer
             * @param[in] parent The corresponding RADAR screen
             */
            NotamOverviewWindow(RadarScreen* parent);

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
