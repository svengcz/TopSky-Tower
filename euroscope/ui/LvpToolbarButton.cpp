/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the LVP toolbar button
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "LvpToolbarButton.h"

using namespace topskytower;
using namespace topskytower::euroscope;

LvpToolbarButton::LvpToolbarButton(RadarScreen* parent) :
        ToolbarButton(parent, "LVP", Gdiplus::RectF(0.0f, 0.0f, 40.0f, 10.0f)) { }

void LvpToolbarButton::clicked() {
    bool isActive = this->active();

    auto configuration = system::ConfigurationRegistry::instance().runtimeConfiguration();
    configuration.lowVisibilityProcedures = !isActive;
    system::ConfigurationRegistry::instance().setRuntimeConfiguration(configuration);
}

bool LvpToolbarButton::active() {
    return system::ConfigurationRegistry::instance().runtimeConfiguration().lowVisibilityProcedures;
}

