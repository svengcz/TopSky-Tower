/*
 * @brief Defines a checkbox
 * @file ui/elements/Checkbox.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <string>

#include "Text.h"
#include "UiElement.h"

namespace topskytower {
    namespace euroscope {
        class RadarScreen;

        /**
         * @brief Defines the checkbox
         * @ingroup euroscope
         */
        class Checkbox : public UiElement {
        private:
            std::string m_title;
            Text        m_titleVisualization;
            bool        m_checked;

        public:
            /**
             * @brief Creates a new checkbox
             * @param[in] parent The parent RADAR screen
             * @param[in] title The title of the checkbox
             * @param[in] dimension The button's dimension
             */
            Checkbox(RadarScreen* parent, const std::string& title, const Gdiplus::RectF& dimension);

            /**
             * @brief Returns if the field is checked or not
             * @return True if the field is checked, else false
             */
            bool checked() const;
            /**
             * @brief Handles the click events
             * @param[in] pt The position of the mouse
             * @param[in] button The clicked mouse button
             * @return True if the click was handled, else false
             */
            bool click(const Gdiplus::PointF& pt, UiManager::MouseButton button) override;
            /**
             * @brief Visualizes the toolbar with all active sub-menus
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            bool visualize(Gdiplus::Graphics* graphics) override;
        };
    }
}
