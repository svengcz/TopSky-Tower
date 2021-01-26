/*
 * @brief Defines the NOTAM overview window
 * @file ui/NotamOverviewWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include "elements/Checkbox.h"
#include "elements/EditText.h"
#include "elements/TableViewer.h"
#include "InsetWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the inset window to give an overview NOTAM
         * @ingroup euroscope
         */
        class NotamOverviewWindow : public InsetWindow {
        private:
            EditText*    m_airportFilter;
            Checkbox*    m_activeFilter;
            TableViewer* m_notamOverview;

            void setOverviewContent();

        public:
            /**
             * @brief Creates a new NOTAM overview viewer
             * @param[in] parent The corresponding RADAR screen
             */
            NotamOverviewWindow(RadarScreen* parent);
            /**
             * @brief Destroys the window
             */
            ~NotamOverviewWindow();

            /**
             * @brief Visualizes the window
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            bool visualize(Gdiplus::Graphics* graphics) override;
        };
    }
}
