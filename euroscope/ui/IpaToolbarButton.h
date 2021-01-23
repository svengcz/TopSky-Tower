/*
 * @brief Defines the IPA toolbar button
 * @file ui/IpaToolbarButton.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include "elements/ToolbarButton.h"
#include "HoppiesLogonWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the toolbar button to control IPA (Independent Parallel Approach)
         * @ingroup euroscope
         */
        class IpaToolbarButton : public ToolbarButton {
        private:
            void clicked();
            bool active();

        public:
            /**
             * @brief Creates a new IPA control button
             * @param[in] parent The parent RADAR screen
             */
            IpaToolbarButton(RadarScreen* parent);
        };
    }
}
