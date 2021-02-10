/*
 * @brief Defines the departure sequence window
 * @file ui/DepartureSequenceWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include "elements/TableViewer.h"
#include "InsetWindow.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the inset window to give an overview NOTAM
         * @ingroup euroscope
         */
        class DepartureSequenceWindow : public InsetWindow {
        private:
            std::list<std::string> m_activeDepartures;
            TableViewer*           m_departureTable;

            bool updateRow(const types::Flight& flight, std::size_t row);

        public:
            /**
             * @brief Creates a new NOTAM overview viewer
             * @param[in] parent The corresponding RADAR screen
             */
            DepartureSequenceWindow(RadarScreen* parent);
            /**
             * @brief Destroys the window
             */
            ~DepartureSequenceWindow();

            /**
             * @brief Visualizes the window
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            bool visualize(Gdiplus::Graphics* graphics) override;
        };
    }
}
