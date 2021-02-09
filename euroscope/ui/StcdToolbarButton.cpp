/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the STCD toolbar button
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "StcdToolbarButton.h"

using namespace topskytower;
using namespace topskytower::euroscope;

StcdToolbarButton::StcdToolbarButton(RadarScreen* parent) :
        ToolbarButton(parent, "STCD", Gdiplus::RectF(0.0f, 0.0f, 60.0f, 10.0f)) { }

void StcdToolbarButton::clicked() {
    bool isActive = this->active();

    auto configuration = system::ConfigurationRegistry::instance().runtimeConfiguration();
    configuration.stcdActive = !isActive;
    system::ConfigurationRegistry::instance().setRuntimeConfiguration(configuration);
}

bool StcdToolbarButton::active() {
    bool isActive = system::ConfigurationRegistry::instance().systemConfiguration().stcdActive;
    isActive &= system::ConfigurationRegistry::instance().runtimeConfiguration().stcdActive;
    return isActive;
}

