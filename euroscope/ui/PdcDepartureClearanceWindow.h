/*
 * @brief Defines the PDC departure clearance window
 * @file ui/PdcDepartureClearanceWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <surveillance/PdcControl.h>

#include "InsetWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the inset window which visualizes PDC messages
         * @ingroup euroscope
         */
        class PdcDepartureClearanceWindow : public InsetWindow {
        private:
            surveillance::PdcControl::ClearanceMessagePtr m_message;

            void centeredPosition();

        public:
            /**
             * @brief Creates a new logon window
             * @param[in] parent The corresponding RADAR screen
             * @param[in] message The clearance message
             */
            PdcDepartureClearanceWindow(RadarScreen* parent, const surveillance::PdcControl::ClearanceMessagePtr& message);
            /**
             * @brief Destroys the window
             */
            ~PdcDepartureClearanceWindow();

            /**
             * @brief Marks the window as active or not
             * @param[in] active True if the window needs to be shown, else false
             */
            void setActive(bool active) override;
            /**
             * @brief Sends the clearance message
             */
            void sendMessage();
        };
    }
}
