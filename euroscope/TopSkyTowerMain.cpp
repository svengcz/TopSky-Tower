/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the entry functions for EuroScope
 */

#include "stdafx.h"

#define CURL_STATICLIB 1
#include <curl/curl.h>

#include "PlugIn.h"

using namespace topskytower::euroscope;

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    static Gdiplus::GdiplusStartupInput __gdiStartupInput;
    static ULONG_PTR                    __gdiplusToken;

    (void)module;
    (void)reserved;

    switch (reason) {
    case DLL_PROCESS_ATTACH:
        Gdiplus::GdiplusStartup(&__gdiplusToken, &__gdiStartupInput, nullptr);
        curl_global_init(CURL_GLOBAL_ALL);
        break;
    case DLL_PROCESS_DETACH:
        curl_global_cleanup();
        Gdiplus::GdiplusShutdown(__gdiplusToken);
        break;
    default:
        break;
    }

    return TRUE;
}

static PlugIn* __plugin = nullptr;

void __declspec(dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance) {
    *ppPlugInInstance = __plugin = new PlugIn();
}

void __declspec(dllexport) EuroScopePlugInExit() {
    if (nullptr != __plugin)
        delete __plugin;
    __plugin = nullptr;
}
