/*
 * @brief Defines an edit text for the user elements
 * @file ui/elements/EditText.h
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
         * @brief Defines the edit text, that can also be used as a password field
         * @ingroup euroscope
         */
        class EditText : public UiElement {
        private:
            std::string    m_headline;
            std::string    m_content;
            bool           m_editable;
            bool           m_passwordField;
            Text           m_headlineVisualization;
            Text           m_contentVisualization;
            Gdiplus::RectF m_editFieldRectangle;

            void uiCallback(const std::string& string);

        public:
            /**
             * @brief Creates a new edit text
             * @param[in] parent The parent RADAR screen
             * @param[in] headline The headline of the edit field
             * @param[in] dimension The button's dimension
             */
            EditText(RadarScreen* parent, const std::string& headline, const Gdiplus::RectF& dimension);

            /**
             * @brief Marks the edit field as a password field or not
             * @param[in] passwordField True if it is an password field, else false
             */
            void setPasswordField(bool passwordField);
            /**
             * @brief Sets the content of the edit field
             * @param[in] content The new content
             */
            void setContent(const std::string& content);
            /**
             * @brief Defines if the edit text is editable
             * @param[in] editable Marks if the edit text is editable or not
             */
            void setEditable(bool editable);
            /**
             * @brief Returns the content of the edit field
             * @return The content of the edit field
             */
            const std::string& content() const;
            /**
             * @brief Sets the position of the element
             * @param[in] position The new position
             */
            void setPosition(const Gdiplus::PointF& position) override;
            /**
             * @brief Moves the position of the element
             * @param[in] direction The direction which is needed to move the element
             */
            void move(const Gdiplus::PointF& direction) override;
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
