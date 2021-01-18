/*
 * @brief Defines the METAR toolbar button
 * @file ui/MetarToolbarButton.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include "elements/ToolbarButton.h"
#include "HoppiesLogonWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the toolbar button to control the METAR viewer
         * @ingroup euroscope
         */
        class MetarToolbarButton : public ToolbarButton {
        private:
            void clicked();
            bool active();

        public:
            /**
             * @brief Creates a new METAR control button
             * @param[in] parent The parent RADAR screen
             */
            MetarToolbarButton(RadarScreen* parent);
        };
    }
}
