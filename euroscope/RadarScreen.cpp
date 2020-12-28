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
        m_controllers(new surveillance::SectorControl()),
        m_flights(new surveillance::FlightRegistry()),
        m_disconnectedFlightsLock(),
        m_disconnectedFlights() { }

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
    if (nullptr != this->m_controllers && true == controller.IsValid() && true == controller.IsController()) {
        std::string_view view(controller.GetPositionId());
        if ("" != view && "XX" != view)
            this->m_controllers->controllerUpdate(Converter::convert(controller));
    }
}

void RadarScreen::OnControllerDisconnect(EuroScopePlugIn::CController controller) {
    if (nullptr != this->m_controllers && true == controller.IsValid() && true == controller.IsController()) {
        std::string_view view(controller.GetPositionId());
        if ("" != view && "XX" != view)
            this->m_controllers->controllerOffline(Converter::convert(controller));
    }
}

void RadarScreen::OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan flightPlan) {
    std::lock_guard guard(this->m_disconnectedFlightsLock);
    this->m_disconnectedFlights.push_back(flightPlan.GetCallsign());
}

void RadarScreen::initialize() {
    if (true == this->m_initialized)
        return;

    auto sctFilename = this->GetPlugIn()->ControllerMyself().GetSectorFileName();

    /* received the correct sector filename identifier */
    if (nullptr != sctFilename && 0 != std::strlen(sctFilename)) {
        formats::EseFileFormat file(sctFilename);

        if (nullptr != this->m_controllers)
            delete this->m_controllers;
        this->m_controllers = new surveillance::SectorControl(this->m_airport, file.sectors());

        if (nullptr != this->m_flights)
            delete this->m_flights;
        this->m_flights = new surveillance::FlightRegistry();

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

    /* remove disconnected flights */
    this->m_disconnectedFlightsLock.lock();
    for (const auto& callsign : std::as_const(this->m_disconnectedFlights)) {
        this->m_flights->removeFlight(callsign);
        this->m_controllers->removeFlight(callsign);
    }
    this->m_disconnectedFlights.clear();
    this->m_disconnectedFlightsLock.unlock();

    auto plugin = static_cast<PlugIn*>(this->GetPlugIn());
    this->m_controllers->setOwnSector(plugin->ControllerMyself().GetPositionId());

    /* update the internal information of the radar targets */
    for (auto rt = plugin->RadarTargetSelectFirst(); true == rt.IsValid(); rt = plugin->RadarTargetSelectNext(rt)) {
        types::Flight flight = Converter::convert(rt, this->m_airport);

        this->m_controllers->update(flight);

        this->m_flights->updateFlight(std::move(flight));
    }
}

const surveillance::FlightRegistry& RadarScreen::flightRegistry() const {
    return *this->m_flights;
}

surveillance::SectorControl& RadarScreen::sectorControl() {
    return *this->m_controllers;
}

const surveillance::SectorControl& RadarScreen::sectorControl() const {
    return *this->m_controllers;
}
