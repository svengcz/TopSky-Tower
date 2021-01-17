/*
 * @brief Defines the UI manager that contains all UI elements
 * @file ui/UiManager.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <list>
#include <map>
#include <string_view>

namespace topskytower {
    namespace euroscope {
        class InsetWindow;
        class RadarScreen;
        class Toolbar;

        /**
         * @brief Defines the UI manager that is defined per RADAR screen
         *
         * The UI manager contains all UI windows and handles it internally which window
         * is active or how click- and  move-events are used.
         */
        class UiManager {
        public:
            /**
             * @brief Defines the mouse button clicks
             */
            enum class MouseButton {
                Left = 1,   /**< Left mouse button is pressed */
                Middle = 2, /**< Middle mouse button is pressed */
                Right = 3  /**< Right mouse button is pressed */
            };

            /**
             * @brief Defines the different window IDs
             */
            enum class WindowId {
                Undefined = 0 /**< An undefined window ID */
            };

        private:
            RadarScreen*                        m_parent;
            Toolbar*                            m_toolbar;
            std::map<std::string, InsetWindow*> m_customWindows;
            std::list<InsetWindow*>             m_renderQueue;

            void updateRenderQueue(InsetWindow* element);
            bool click(InsetWindow* element, const Gdiplus::PointF& pt, MouseButton button);
            bool move(InsetWindow* element, const Gdiplus::PointF& pt, bool released);

        public:
            /**
             * @brief Creates a new UI manager that is responsible for the corresponding RADAR screen
             * @param[in] parent The corresponding RADAR screen
             */
            UiManager(RadarScreen* parent);
            /**
             * @brief Destroys the manager and all internal structures
             */
            ~UiManager();

            /**
             * @brief Activates an UI element
             * @param[in] id The requested UI element
             */
            void activateUi(WindowId id);
            /**
             * @brief Handles the click events
             * @param[in] objectName The name of the clicked element
             * @param[in] pt The position of the mouse
             * @param[in] button The clicked mouse button
             * @return True if the click was handled, else false
             */
            bool click(const std::string_view& objectName, const Gdiplus::PointF& pt, MouseButton button);
            /**
             * @brief Moves the position of the element
             * @param[in] objectName The name of the moved element
             * @param[in] pt The position of the mouse at the click event
             * @param[in] released Indicates if the left mouse button is released or not
             * @return True if the move was handled, else false
             */
            bool move(const std::string_view& objectName, const Gdiplus::PointF& pt, bool released);
            /**
             * @brief Checks if a window is active or not
             * @param[in] name The window's name
             * @return True if the window is active, else false
             */
            bool windowIsActive(const std::string& name) const;
            /**
             * @brief Adds a custom window into the manager
             * @param[in] window The custom window
             */
            void addCustomWindow(InsetWindow* window);
            /**
             * @brief Removes a custom window into the manager
             * @param[in] window The custom window
             */
            void removeCustomWindow(InsetWindow* window);
            /**
             * @brief Resets all internal states if no UI-handled element was clicked or moved
             */
            void resetClickStates();
            /**
             * @brief Visualizes the toolbar with all active sub-menus
             * @param[in] graphics The graphics container
             */
            void visualize(Gdiplus::Graphics* graphics);
        };
    }
}
