/*
 * @brief Defines a button for the toolbar
 * @file ui/elements/ToolbarButton.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>

#include "Text.h"
#include "UiElement.h"

namespace topskytower {
    namespace euroscope {
        class RadarScreen;

        /**
         * @brief Defines the toolbar button which also visualizes an internal status
         * @ingroup euroscope
         */
        class ToolbarButton : public UiElement {
        private:
            std::string m_text;
            Text        m_visualization;

        protected:
            /**
             * @brief Called after the button was clicked
             */
            virtual void clicked() = 0;
            /**
             * @brief Checks if the button is indicated as active
             */
            virtual bool active() = 0;

        public:
            /**
             * @brief Creates a new button
             * @param[in] parent The parent RADAR screen
             * @param[in] text The text inside the button
             * @param[in] dimension The button's dimension
             */
            ToolbarButton(RadarScreen* parent, const std::string& text, const Gdiplus::RectF& dimension);
            /**
             * @brief Destroys the button
             */
            virtual ~ToolbarButton() { }

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
