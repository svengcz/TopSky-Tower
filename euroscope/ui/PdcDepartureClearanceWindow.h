/*
 * @brief Defines the PDC departure clearance window
 * @file ui/PdcDepartureClearanceWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <management/PdcControl.h>

#include "elements/EditText.h"
#include "InsetWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the inset window which visualizes PDC messages
         * @ingroup euroscope
         */
        class PdcDepartureClearanceWindow : public InsetWindow {
        private:
            management::PdcControl::ClearanceMessagePtr m_message;
            EditText*                                   m_nextFrequencyField;

            void centeredPosition();

        public:
            /**
             * @brief Creates a new logon window
             * @param[in] parent The corresponding RADAR screen
             * @param[in] message The clearance message
             */
            PdcDepartureClearanceWindow(RadarScreen* parent, const management::PdcControl::ClearanceMessagePtr& message);
            /**
             * @brief Destroys the window
             */
            ~PdcDepartureClearanceWindow();

            /**
             * @brief Handles the click events
             * @param[in] pt The position of the mouse
             * @param[in] button The clicked mouse button
             * @return True if the click was handled, else false
             */
            bool click(const Gdiplus::PointF& pt, UiManager::MouseButton button) override;
            /**
             * @brief Sends the clearance message
             */
            void sendMessage();
            /**
             * @brief Marks the window as active or not
             * @param[in] active True if the window needs to be shown, else false
             */
            void setActive(bool active) override;
            /**
             * @brief Visualizes the toolbar with all active sub-menus
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            bool visualize(Gdiplus::Graphics* graphics) override;
        };
    }
}
