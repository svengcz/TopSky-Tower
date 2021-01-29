/*
 * @brief Defines the LVP toolbar button
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
         * @brief Defines the toolbar button to control LVP (low-visibility procedures)
         * @ingroup euroscope
         */
        class LvpToolbarButton : public ToolbarButton {
        private:
            void clicked();
            bool active();

        public:
            /**
             * @brief Creates a new LVP control button
             * @param[in] parent The parent RADAR screen
             */
            LvpToolbarButton(RadarScreen* parent);
        };
    }
}
