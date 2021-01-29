/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the PDC toolbar button
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <management/PdcControl.h>

#include "../RadarScreen.h"
#include "PdcToolbarButton.h"

using namespace topskytower::euroscope;

PdcToolbarButton::PdcToolbarButton(RadarScreen* parent) :
        ToolbarButton(parent, "PDC", Gdiplus::RectF(0.0f, 0.0f, 40.0f, 10.0f)),
        m_window(nullptr) { }

void PdcToolbarButton::clicked() {
    bool isActive = this->active();

    if (true == isActive) {
        management::PdcControl::instance().removeAirport(this->m_parent->airportIcao());
    }
    else if (false == this->m_parent->uiManager().windowIsActive("PDC")) {
        this->m_window = new HoppiesLogonWindow(this->m_parent);
        this->m_window->setActive(true);
    }
}

bool PdcToolbarButton::active() {
    return management::PdcControl::instance().airportLoggedIn(this->m_parent->airportIcao());
}

