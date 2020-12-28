/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * License:
 *   LGPLv3
 * Brief:
 *   Implements the RADAR display of EuroScope
 */

#include "stdafx.h"

#include <gdiplus.h>

#include <formats/EseFileFormat.h>

#include "Converter.h"
#include "PlugIn.h"
#include "RadarScreen.h"

using namespace topskytower;
using namespace topskytower::euroscope;

RadarScreen::RadarScreen() :
        EuroScopePlugIn::CRadarScreen(),
        m_initialized(false),
        m_airport(),
        m_controller(new surveillance::Controller()) { }

RadarScreen::~RadarScreen() { }

void RadarScreen::OnAsrContentLoaded(bool loaded) {
    if (true == loaded) {
        auto value = this->GetDataFromAsr("Airport");
        if (nullptr != value)
            this->m_airport = value;
    }
}

void RadarScreen::OnAsrContentToBeClosed() {
    delete this;
}

void RadarScreen::OnControllerPositionUpdate(EuroScopePlugIn::CController controller) {
    if (nullptr != this->m_controller && true == controller.IsValid() && true == controller.IsController()) {
        std::string_view view(controller.GetPositionId());
        if ("" != view && "XX" != view)
            this->m_controller->controllerUpdate(Converter::convert(controller));
    }
}

void RadarScreen::OnControllerDisconnect(EuroScopePlugIn::CController controller) {
    if (nullptr != this->m_controller && true == controller.IsValid() && true == controller.IsController()) {
        std::string_view view(controller.GetPositionId());
        if ("" != view && "XX" != view)
            this->m_controller->controllerOffline(Converter::convert(controller));
    }
}

void RadarScreen::initialize() {
    if (true == this->m_initialized)
        return;

    auto sctFilename = this->GetPlugIn()->ControllerMyself().GetSectorFileName();

    /* received the correct sector filename identifier */
    if (nullptr != sctFilename && 0 != std::strlen(sctFilename)) {
        formats::EseFileFormat file(sctFilename);

        if (nullptr != this->m_controller)
            delete this->m_controller;
        this->m_controller = new surveillance::Controller(this->m_airport, file.sectors());

        this->m_initialized = true;
    }
}

void RadarScreen::OnRefresh(HDC hdc, int phase) {
    (void)hdc;

    if (EuroScopePlugIn::REFRESH_PHASE_BEFORE_TAGS != phase)
        return;

    /* check if we need to initialize the system */
    this->initialize();
    if (false == this->m_initialized)
        return;

    auto plugin = static_cast<PlugIn*>(this->GetPlugIn());
    this->m_controller->setOwnSector(plugin->ControllerMyself().GetPositionId());

    /* update the internal information of the radar targets */
    for (auto rt = plugin->RadarTargetSelectFirst(); true == rt.IsValid(); rt = plugin->RadarTargetSelectNext(rt)) {
        types::Flight flight = Converter::convert(rt, this->m_airport);

        this->m_controller->update(flight);
    }
}

surveillance::Controller& RadarScreen::controllerManager() {
    return *this->m_controller;
}

const surveillance::Controller& RadarScreen::controllerManager() const {
    return *this->m_controller;
}
