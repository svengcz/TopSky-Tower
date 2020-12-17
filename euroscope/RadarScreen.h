/*
 * @brief Defines the EuroScope RADAR screen
 * @file euroscope/RadarScreen.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#pragma warning(push, 0)
#include <EuroScopePlugIn.h>
#pragma warning(pop)

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the RADAR screen
         * @ingroup euroscope
         */
        class RadarScreen : public EuroScopePlugIn::CRadarScreen {
        public:
            /**
             * @brief Creates a new RADAR screen
             */
            RadarScreen();
            /**
             * @brief Destroys all internal structures
             */
            ~RadarScreen();
        };
    }
}
