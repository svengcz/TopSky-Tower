/*
 * @brief Defines the base class of all UI elements
 * @file ui/elements/UiElement.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>

#include "../UiManager.h"

namespace topskytower {
    namespace euroscope {
        class RadarScreen;

        /**
         * @brief Defines the base class of all UI elements
         * @ingroup euroscope
         */
        class UiElement {
        protected:
            RadarScreen*   m_parent;
            Gdiplus::RectF m_area;

            /**
             * @brief Checks if a point is inside a rectangle
             * @param[in] pt The requested position
             * @param[in] rectangle The checked rectangle
             * @return True if the point is in the rectangle, else false
             */
            static bool isInRectangle(const Gdiplus::PointF& pt, const Gdiplus::RectF& rectangle);
            /**
             * @brief Returns the background color based on the configuration
             * @return The background color
             */
            static Gdiplus::Color backgroundColor();
            /**
             * @brief Returns the foreground color based on the configuration
             * @return The foreground color
             */
            static Gdiplus::Color foregroundColor();

        public:
            /**
             * @brief Creates a new UI element
             * @param[in] parent The parent RADAR screen
             * @param[in] rectangle The dimension of the element
             */
            UiElement(RadarScreen* parent, const Gdiplus::RectF& rectangle);
            /**
             * @brief Destroys the element
             */
            virtual ~UiElement() { }

            /**
             * @brief Sets the position of the element
             * @param[in] position The new position
             */
            virtual void setPosition(const Gdiplus::PointF& position);
            /**
             * @brief Moves the position of the element
             * @param[in] direction The direction which is needed to move the element
             */
            virtual void move(const Gdiplus::PointF& direction);
            /**
             * @brief Handles the click events
             * @param[in] pt The position of the mouse
             * @param[in] button The clicked mouse button
             * @return True if the click was handled, else false
             */
            virtual bool click(const Gdiplus::PointF& pt, UiManager::MouseButton button) = 0;
            /**
             * @brief Visualizes the toolbar with all active sub-menus
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            virtual bool visualize(Gdiplus::Graphics* graphics) = 0;
            /**
             * @brief Sets the new area of the element
             * @param[in] area the new area dimension
             */
            void setArea(Gdiplus::RectF& area);
            /**
             * @brief Returns the area of the elment
             * @return The element's area
             */
            const Gdiplus::RectF& area() const;
        };
    }
}
