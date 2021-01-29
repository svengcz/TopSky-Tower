/*
 * @brief Defines the MTCD toolbar button
 * @file ui/MtcdToolbarButton.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include "elements/ToolbarButton.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the toolbar button to control MTCD
         * @ingroup euroscope
         */
        class MtcdToolbarButton : public ToolbarButton {
        private:
            void clicked();
            bool active();

        public:
            /**
             * @brief Creates a new MTCD control button
             * @param[in] parent The parent RADAR screen
             */
            MtcdToolbarButton(RadarScreen* parent);
        };
    }
}
