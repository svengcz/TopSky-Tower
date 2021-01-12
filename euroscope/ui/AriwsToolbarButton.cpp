/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the ARIWS toolbar button
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
    system::ConfigurationRegistry::instance().runtimeConfiguration().ariwsActive = !isActive;
}

bool AriwsToolbarButton::active() {
    bool isActive = system::ConfigurationRegistry::instance().systemConfiguration().ariwsActive;
    isActive &= system::ConfigurationRegistry::instance().runtimeConfiguration().ariwsActive;
    return isActive;
}
