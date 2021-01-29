/*
 * @brief Defines the generic message viewer window
 * @file ui/MessageViewerWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include "InsetWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the inset window which visualizes generic messages
         * @ingroup euroscope
         */
        class MessageViewerWindow : public InsetWindow {
        private:
            bool m_firstRendering;

            void centeredPosition();

        public:
            /**
             * @brief Creates a new PDC message viewer
             * @param[in] parent The corresponding RADAR screen
             * @param[in] title The window's title
             * @param[in] message The message
             */
            MessageViewerWindow(RadarScreen* parent, const std::string& title, const std::string& message);
            /**
             * @brief Destroys the window
             */
            ~MessageViewerWindow();

            /**
             * @brief Visualizes the toolbar with all active sub-menus
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            bool visualize(Gdiplus::Graphics* graphics) override;
        };
    }
}
