/*
 * @brief Defines a viewer to visualize texts
 * @file ui/elements/TextViewer.h
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
         * @brief Defines the text viewer
         * @ingroup euroscope
         */
        class TextViewer : public UiElement {
        private:
            std::string m_text;
            Text        m_visualization;

        public:
            /**
             * @brief Creates a new text viewer
             * @param[in] parent The parent RADAR screen
             * @param[in] text The text inside the viewer
             * @param[in] dimension The viewer's dimension
             */
            TextViewer(RadarScreen* parent, const std::string& text, const Gdiplus::RectF& dimension);
            /**
             * @brief Destroys the button
             */
            virtual ~TextViewer() { }

            /**
             * @brief Sets a new text for the viewer
             * @param[in] text The new text
             */
            void setText(const std::string& text);
            /**
             * @brief Handles the click events
             * @param[in] pt The position of the mouse
             * @param[in] button The clicked mouse button
             * @return True if the click was handled, else false
             */
            bool click(const Gdiplus::PointF& pt, UiManager::MouseButton button) override;
            /**
             * @brief Prepares the widget visualization
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            bool prepareVisualization(Gdiplus::Graphics* graphics);
            /**
             * @brief Visualizes the toolbar with all active sub-menus
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            bool visualize(Gdiplus::Graphics* graphics) override;
        };
    }
}
