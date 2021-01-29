/*
 * @brief Defines the PDC toolbar button
 * @file ui/PdcToolbarButton.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include "elements/ToolbarButton.h"
#include "HoppiesLogonWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the toolbar button to control PDC
         * @ingroup euroscope
         */
        class PdcToolbarButton : public ToolbarButton {
        private:
            HoppiesLogonWindow* m_window;

            void clicked();
            bool active();

        public:
            /**
             * @brief Creates a new PDC control button
             * @param[in] parent The parent RADAR screen
             */
            PdcToolbarButton(RadarScreen* parent);
        };
    }
}
