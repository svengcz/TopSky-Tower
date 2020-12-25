/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * License:
 *   LGPLv3
 * Brief:
 *   Implements the entry functions for EuroScope
 */

#include "stdafx.h"

#include <gdiplus.h>

#include "PlugIn.h"
#include "MFCApplication.h"

using namespace topskytower::euroscope;

BEGIN_MESSAGE_MAP(Application, CWinApp)
END_MESSAGE_MAP()

static Gdiplus::GdiplusStartupInput __gdiStartupInput;
static ULONG_PTR                    __gdiplusToken;

Application::Application() { }

Application app;

BOOL Application::InitInstance() {
    CWinApp::InitInstance();
    return TRUE;
}

PlugIn* plugin = nullptr;

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance) {
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Gdiplus::GdiplusStartup(&__gdiplusToken, &__gdiStartupInput, nullptr);

    plugin = new PlugIn();
    *ppPlugInInstance = plugin;
}

void __declspec(dllexport) EuroScopePlugInExit() {
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    Gdiplus::GdiplusShutdown(__gdiplusToken);
}
