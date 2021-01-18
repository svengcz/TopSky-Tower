/*
 * @brief Defines the METAR viewer window
 * @file ui/MetarViewerWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include "InsetWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the inset window which visualizes generic messages
         * @ingroup euroscope
         */
        class MetarViewerWindow : public InsetWindow {
        private:
            using TimePoint = std::chrono::system_clock::time_point;

            struct QnhTracker {
                std::uint16_t  qnhValue;
                TimePoint      blinkUntil;
                Gdiplus::Color currentColor;
                TimePoint      lastSwitch;
            };

            std::map<std::string, QnhTracker> m_qnhValues;

            void updateData();

        public:
            /**
             * @brief Creates a new METAR viewer
             * @param[in] parent The corresponding RADAR screen
             */
            MetarViewerWindow(RadarScreen* parent);
            /**
             * @brief Destroys the window
             */
            ~MetarViewerWindow();

            /**
             * @brief Visualizes the toolbar with all active sub-menus
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            bool visualize(Gdiplus::Graphics* graphics) override;
        };
    }
}
