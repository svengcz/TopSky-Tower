/*
 * @brief Defines the PDC message viewer window
 * @file ui/PdcMessageViewerWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <gdiplus.h>

#include <surveillance/PdcControl.h>

#include "MessageViewerWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the inset window which visualizes PDC messages
         * @ingroup euroscope
         */
        class PdcMessageViewerWindow : public MessageViewerWindow {
        public:
            /**
             * @brief Creates a new PDC message viewer
             * @param[in] parent The corresponding RADAR screen
             * @param[in] message The PDC message
             */
            PdcMessageViewerWindow(RadarScreen* parent, const surveillance::PdcControl::MessagePtr& message);
        };
    }
}
