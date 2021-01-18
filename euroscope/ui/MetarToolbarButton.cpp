/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the METAR toolbar button
 */

#include "stdafx.h"

#include "../RadarScreen.h"
#include "MetarToolbarButton.h"
#include "MetarViewerWindow.h"

using namespace topskytower::euroscope;

MetarToolbarButton::MetarToolbarButton(RadarScreen* parent) :
        ToolbarButton(parent, "METAR", Gdiplus::RectF(0.0f, 0.0f, 50.0f, 10.0f)) { }

void MetarToolbarButton::clicked() {
    bool isActive = this->active();

    if (false == isActive) {
        auto viewer = new MetarViewerWindow(this->m_parent);
        viewer->setActive(true);
    }
}

bool MetarToolbarButton::active() {
    return this->m_parent->uiManager().windowIsActive("METAR");
}

