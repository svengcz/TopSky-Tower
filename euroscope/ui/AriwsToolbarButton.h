/*
 * @brief Defines the ARIWS toolbar button
 * @file ui/AriwsToolbarButton.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include "elements/ToolbarButton.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the toolbar button to control ARIWS
         * @ingroup euroscope
         */
        class AriwsToolbarButton : public ToolbarButton {
        private:
            void clicked();
            bool active();

        public:
            /**
             * @brief Creates a new ARIWS control button
             * @param[in] parent The parent RADAR screen
             */
            AriwsToolbarButton(RadarScreen* parent);
        };
    }
}
