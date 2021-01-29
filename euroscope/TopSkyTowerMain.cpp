/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the entry functions for EuroScope
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include "PlugIn.h"

using namespace topskytower::euroscope;

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    (void)module;
    (void)reserved;

    switch (reason) {
    case DLL_PROCESS_ATTACH:
    case DLL_PROCESS_DETACH:
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
