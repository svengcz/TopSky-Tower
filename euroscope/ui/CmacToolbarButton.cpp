/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the CMAC toolbar button
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "CmacToolbarButton.h"

using namespace topskytower;
using namespace topskytower::euroscope;

CmacToolbarButton::CmacToolbarButton(RadarScreen* parent) :
        ToolbarButton(parent, "CMAC", Gdiplus::RectF(0.0f, 0.0f, 60.0f, 10.0f)) { }

void CmacToolbarButton::clicked() {
    bool isActive = this->active();

    auto configuration = system::ConfigurationRegistry::instance().runtimeConfiguration();
    configuration.cmacActive = !isActive;
    system::ConfigurationRegistry::instance().setRuntimeConfiguration(configuration);
}

bool CmacToolbarButton::active() {
    bool isActive = system::ConfigurationRegistry::instance().systemConfiguration().cmacActive;
    isActive &= system::ConfigurationRegistry::instance().runtimeConfiguration().cmacActive;
    return isActive;
}
