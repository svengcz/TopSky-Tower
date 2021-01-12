/*
 * @brief Defines the ARIWS toolbar button
 * @file ui/AriwsToolbarButton.h
 * @author Sven Czarnian <devel@svcz.de>
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
