/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the IPA toolbar button
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "IpaToolbarButton.h"

using namespace topskytower;
using namespace topskytower::euroscope;

IpaToolbarButton::IpaToolbarButton(RadarScreen* parent) :
        ToolbarButton(parent, "IPA", Gdiplus::RectF(0.0f, 0.0f, 40.0f, 10.0f)) { }

void IpaToolbarButton::clicked() {
    bool isActive = this->active();

    auto configuration = system::ConfigurationRegistry::instance().runtimeConfiguration();
    configuration.ipaActive = !isActive;
    system::ConfigurationRegistry::instance().setRuntimeConfiguration(configuration);
}

bool IpaToolbarButton::active() {
    return system::ConfigurationRegistry::instance().runtimeConfiguration().ipaActive;
}

