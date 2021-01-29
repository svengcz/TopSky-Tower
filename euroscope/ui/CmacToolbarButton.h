/*
 * @brief Defines the CMAC toolbar button
 * @file ui/CmacToolbarButton.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include "elements/ToolbarButton.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the toolbar button to control CMAC
         * @ingroup euroscope
         */
        class CmacToolbarButton : public ToolbarButton {
        private:
            void clicked();
            bool active();

        public:
            /**
             * @brief Creates a new CMAC control button
             * @param[in] parent The parent RADAR screen
             */
            CmacToolbarButton(RadarScreen* parent);
        };
    }
}
