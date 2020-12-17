/*
 * @brief Defines the plug-ins entry point
 * @file euroscope/MFCApplication.h
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
         * @brief Defines the entry points of the plug-in
         * @ingroup euroscope
         */
        class Application : public CWinApp {
        public:
            /**
             * @brief Creates a new plug-in
             */
            Application();

            /**
             * @brief Initializes the plug-in
             * @return TRUE if the initialization was succesful, else false
             */
            BOOL InitInstance() override;
            /**< Message handling back-end */
            DECLARE_MESSAGE_MAP()
        };
    }
}
