/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the CMAC toolbar button
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
    system::ConfigurationRegistry::instance().runtimeConfiguration().cmacActive = !isActive;
}

bool CmacToolbarButton::active() {
    bool isActive = system::ConfigurationRegistry::instance().systemConfiguration().cmacActive;
    isActive &= system::ConfigurationRegistry::instance().runtimeConfiguration().cmacActive;
    return isActive;
}
