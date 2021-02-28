/*
 * @brief Defines the Hoppies logon inset window
 * @file ui/HoppiesLogonWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include "InsetWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the inset window which handles the logon-information of hoppies
         * @ingroup euroscope
         */
        class HoppiesLogonWindow : public InsetWindow {
        public:
            /**
             * @brief Creates a new logon window
             * @param[in] parent The corresponding RADAR screen
             */
            HoppiesLogonWindow(RadarScreen* parent);

            /**
             * @brief Marks the window as active or not
             * @param[in] active True if the window needs to be shown, else false
             */
            void setActive(bool active) override;
            /**
             * @brief Returns the station for hoppies
             * @return The stations code
             */
            const std::string& station() const;
            /**
             * @brief Returns the Hoppies-code
             * @return The Hoppies-logon-code
             */
            const std::string& code() const;
        };
    }
}
