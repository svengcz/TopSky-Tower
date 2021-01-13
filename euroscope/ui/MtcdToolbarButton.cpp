/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the MTCD toolbar button
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
    system::ConfigurationRegistry::instance().runtimeConfiguration().mtcdActive = !isActive;
}

bool MtcdToolbarButton::active() {
    bool isActive = system::ConfigurationRegistry::instance().systemConfiguration().mtcdActive;
    isActive &= system::ConfigurationRegistry::instance().runtimeConfiguration().mtcdActive;
    return isActive;
}
