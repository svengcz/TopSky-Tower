/*
 * @brief Defines an drop down menu for the user elements
 * @file ui/elements/DropDownMenu.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <list>
#include <string>

#include "Text.h"
#include "UiElement.h"

namespace topskytower {
    namespace euroscope {
        class RadarScreen;

        /**
         * @brief Defines the drop down menu
         * @ingroup euroscope
         */
        class DropDownMenu : public UiElement {
        private:
            std::string            m_headline;
            std::string            m_content;
            Text                   m_headlineVisualization;
            Text                   m_contentVisualization;
            Gdiplus::RectF         m_fieldRectangle;
            std::list<std::string> m_elements;

            void uiCallback(const std::string& string);

        public:
            /**
             * @brief Creates a new drop down menu
             * @param[in] parent The parent RADAR screen
             * @param[in] headline The headline of the edit field
             * @param[in] dimension The button's dimension
             */
            DropDownMenu(RadarScreen* parent, const std::string& headline, const Gdiplus::RectF& dimension);

            /**
             * @brief Sets the entries of the drop down menu
             * @param[in] entries All entries of the menu
             */
            void setEntries(std::initializer_list<std::string> entries);
            /**
             * @brief Adds a new entry to the drop down list
             * @param[in] entry The new entry
             */
            void addEntry(const std::string& entry);
            /**
             * @brief Returns the content of the edit field
             * @return The selected drop down element
             */
            const std::string& selected() const;
            /**
             * @brief Returns the selected index
             * @return The clicked index
             */
            int selectedIndex() const;
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
             * @brief Visualizes the drop down menu
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            bool visualize(Gdiplus::Graphics* graphics) override;
        };
    }
}
