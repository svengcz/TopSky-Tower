/*
 * @brief Defines the PDC message viewer window
 * @file ui/PdcMessageViewerWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <gdiplus.h>

#include <surveillance/PdcControl.h>

#include "InsetWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the inset window which visualizes PDC messages
         * @ingroup euroscope
         */
        class PdcMessageViewerWindow : public InsetWindow {
        private:
            bool m_firstRendering;

            void centeredPosition();

        public:
            /**
             * @brief Creates a new logon window
             * @param[in] parent The corresponding RADAR screen
             * @param[in[ message The PDC message
             */
            PdcMessageViewerWindow(RadarScreen* parent, const surveillance::PdcControl::MessagePtr& message);
            /**
             * @brief Destroys the window
             */
            ~PdcMessageViewerWindow();

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
