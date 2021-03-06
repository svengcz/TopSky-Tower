/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the ARIWS toolbar button
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "AriwsToolbarButton.h"

using namespace topskytower;
using namespace topskytower::euroscope;

AriwsToolbarButton::AriwsToolbarButton(RadarScreen* parent) :
        ToolbarButton(parent, "ARIWS", Gdiplus::RectF(0.0f, 0.0f, 60.0f, 10.0f)) { }

void AriwsToolbarButton::clicked() {
    bool isActive = this->active();

    auto configuration = system::ConfigurationRegistry::instance().runtimeConfiguration();
    configuration.ariwsActive = !isActive;
    system::ConfigurationRegistry::instance().setRuntimeConfiguration(configuration);
}

bool AriwsToolbarButton::active() {
    bool isActive = system::ConfigurationRegistry::instance().systemConfiguration().ariwsActive;
    isActive &= system::ConfigurationRegistry::instance().runtimeConfiguration().ariwsActive;
    return isActive;
}
