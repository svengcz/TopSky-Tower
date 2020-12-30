/*
 * @brief Defines the generic inset window
 * @file ui/InsetWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <list>
#include <string_view>

#include <gdiplus.h>

#include "elements/Text.h"
#include "elements/UiElement.h"

namespace topskytower {
    namespace euroscope {
        class RadarScreen;

        /**
         * @brief Defines the inset window which will be shown on top of the screen
         * @ingroup euroscope
         */
        class InsetWindow : public UiElement {
        private:
            bool            m_active;
            bool            m_resizable;
            bool            m_resizeActive;
            std::string     m_title;
            Text            m_titleVisualization;
            Gdiplus::RectF  m_headlineRectangle;
            Gdiplus::RectF  m_crossRectangle;
            Gdiplus::RectF  m_resizeRectangle;
            Gdiplus::PointF m_offsetToCenter;

            void move(const Gdiplus::PointF& direction) override;

        protected:
            Gdiplus::SizeF        m_minimumSize; /**< The minimum size of the element */
            Gdiplus::RectF        m_contentArea; /**< The area that can be used by the content */
            std::list<UiElement*> m_elements;    /**< All elements of the window */

            /**
             * @brief Creates a new window
             * @param[in] title The window's title
             * @param[in] parent The parent RADAR screen
             * @param[in] rectangle The position and dimension of the window
             * @param[in] resizable Marks if the window is resizable or not
             */
            InsetWindow(const std::string& title, RadarScreen* parent, const Gdiplus::RectF& rectangle, bool resizable);

            /**
             * @brief Sets the content area's size
             * @param[in] size The new dimension
             */
            void setContentSize(const Gdiplus::SizeF size);
            /**
             * @brief The function is called after a resize-event triggered
             */
            virtual void resize();

        public:
            /**
             * @brief Destroys the element
             */
            virtual ~InsetWindow();

            /**
             * @brief Returns the window's title
             * @return The title
             */
            const std::string& title() const;
            /**
             * @brief Marks the window as active or not
             * @param[in] active True if the window needs to be shown, else false
             */
            virtual void setActive(bool active);
            /**
             * @brief Checks if the window is active
             * @return True if the window is active, else false
             */
            bool isActive() const;
            /**
             * @brief Sets the position of the element
             * @param[in] position The new position
             */
            void setPosition(const Gdiplus::PointF& position) override;
            /**
             * @brief Handles the click events
             * @param[in] pt The position of the mouse
             * @param[in] button The clicked mouse button
             * @return True if the click was handled, else false
             */
            virtual bool click(const Gdiplus::PointF& pt, UiManager::MouseButton button) override;
            /**
             * @brief Moves the window if it is needed
             * @param[in] pt The position of the mouse
             * @param[in] released True if the mouse button is released, else false
             * @return True if the window is moved, else false
             */
            virtual bool move(const Gdiplus::PointF& pt, bool released);
            /**
             * @brief Visualizes the toolbar with all active sub-menus
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            virtual bool visualize(Gdiplus::Graphics* graphics) override;
        };
    }
}
