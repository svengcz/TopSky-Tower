/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the version checker
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"
#include "HiddenWindow.h"
#include "PlugIn.h"

static topskytower::euroscope::PlugIn* __plugin = nullptr;

LRESULT CALLBACK HiddenWindow(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    COPYDATASTRUCT* data;

    switch (msg) {
    case WM_CREATE:
        __plugin = reinterpret_cast<topskytower::euroscope::PlugIn*>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);
        return TRUE;
    case WM_COPYDATA:
        data = reinterpret_cast<COPYDATASTRUCT*>(lparam);

        if (nullptr != data && 666 == data->dwData && nullptr != data->lpData && nullptr != __plugin) {
            int b = 0;
        }

        return TRUE;
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}
