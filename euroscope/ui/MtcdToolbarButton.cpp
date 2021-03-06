/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the MTCD toolbar button
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "MtcdToolbarButton.h"

using namespace topskytower;
using namespace topskytower::euroscope;

MtcdToolbarButton::MtcdToolbarButton(RadarScreen* parent) :
        ToolbarButton(parent, "MTCD", Gdiplus::RectF(0.0f, 0.0f, 60.0f, 10.0f)) { }

void MtcdToolbarButton::clicked() {
    bool isActive = this->active();

    auto configuration = system::ConfigurationRegistry::instance().runtimeConfiguration();
    configuration.mtcdActive = !isActive;
    system::ConfigurationRegistry::instance().setRuntimeConfiguration(configuration);
}

bool MtcdToolbarButton::active() {
    bool isActive = system::ConfigurationRegistry::instance().systemConfiguration().mtcdActive;
    isActive &= system::ConfigurationRegistry::instance().runtimeConfiguration().mtcdActive;
    return isActive;
}
