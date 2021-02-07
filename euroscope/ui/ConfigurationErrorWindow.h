/*
 * @brief Defines the window to visualize configuration errors
 * @file ui/ConfigurationErrorWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include "MessageViewerWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the inset window which visualizes configuration errors
         * @ingroup euroscope
         */
        class ConfigurationErrorWindow : public MessageViewerWindow {
        public:
            /**
             * @brief Creates a new PDC message viewer
             * @param[in] parent The corresponding RADAR screen
             * @param[in] title The window's title
             * @param[in] message The message
             */
            ConfigurationErrorWindow(RadarScreen* parent);
        };
    }
}
