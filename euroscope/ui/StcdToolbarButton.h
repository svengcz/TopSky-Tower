/*
 * @brief Defines the STCD toolbar button
 * @file ui/StcdToolbarButton.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include "elements/ToolbarButton.h"
#include "HoppiesLogonWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the toolbar button to control STCD (Short Term Conflict Detection)
         * @ingroup euroscope
         */
        class StcdToolbarButton : public ToolbarButton {
        private:
            void clicked();
            bool active();

        public:
            /**
             * @brief Creates a new STCD control button
             * @param[in] parent The parent RADAR screen
             */
            StcdToolbarButton(RadarScreen* parent);
        };
    }
}
