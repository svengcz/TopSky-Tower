/*
 * @brief Defines the toolbar
 * @file ui/Toolbar.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <list>

#include "elements/UiElement.h"
#include "elements/Text.h"
#include "elements/ToolbarButton.h"

namespace topskytower {
    namespace euroscope {
        class RadarScreen;
        class UiManager;

        /**
         * @brief Defines the toolbar which will be shown on top of the screen
         * @ingroup euroscope
         *
         * The toolbar is shown on top and is used to give more convenient ways to control
         * the complete plug-in.
         */
        class Toolbar : public UiElement {
        private:
            struct Level;

            enum class ClickId {
                Undefined       = 0,
                Group           = 1,
                Settings        = 2,
                Reload          = 3,
                ReloadSystem    = 4,
                ReloadAircrafts = 5,
                ReloadAirports  = 6,
                ReloadEvents    = 7,
                Windows         = 8,
                Notams          = 9,
                Events          = 10
            };

            struct Element {
                ClickId                clickId;
                std::string            text;
                Text                   visualization;
                Gdiplus::RectF         rectangle;
                std::shared_ptr<Level> child;

                Element() :
                        clickId(ClickId::Undefined),
                        text(),
                        visualization(),
                        rectangle(),
                        child() { }
            };

            struct Level {
                bool               active;
                std::list<Element> elements;
                Gdiplus::RectF     rectangle;

                Level() :
                        active(false),
                        elements(),
                        rectangle() { }
            };

            UiManager*                m_manager;
            std::shared_ptr<Level>    m_toplevel;
            std::list<ToolbarButton*> m_buttons;

            static void createElement(const std::string& text, ClickId uid, std::shared_ptr<Level>& parent);
            static ClickId findClickedElement(const std::shared_ptr<Level>& level, const Gdiplus::PointF& pt);
            static void deactivate(std::shared_ptr<Level>& level);
            int drawLevel(Gdiplus::Graphics* graphics, std::shared_ptr<Level>& level, int startX, int startY, bool horizontal);
            void draw(Gdiplus::Graphics* graphics, std::shared_ptr<Level>& level, int startX, int startY, bool horizontal);
            void setPosition(const Gdiplus::PointF& position) override;
            void move(const Gdiplus::PointF& direction) override;
            void initialize();

        public:
            /**
             * @brief Creates the toolbar
             * @param[in] parent The parent RADAR screen
             * @param[in] manager The window manager
             */
            Toolbar(RadarScreen* parent, UiManager* manager);
            /**
             * @brief Destroys all internal elements
             */
            ~Toolbar();

            /**
             * @brief Handles the click events
             * @param[in] pt The position of the mouse
             * @param[in] button The clicked mouse button
             * @return True if the click was handled, else false
             */
            bool click(const Gdiplus::PointF& pt, UiManager::MouseButton button) override;
            /**
             * @brief Deactivates the visualization of the sub-menus
             */
            void resetClickStates();
            /**
             * @brief Visualizes the toolbar with all active sub-menus
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            bool visualize(Gdiplus::Graphics* graphics) override;
        };
    }
}
