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

#include <formats/AirportFileFormat.h>
#include <formats/EseFileFormat.h>
#include <surveillance/PdcControl.h>

#include "Converter.h"
#include "PlugIn.h"
#include "RadarScreen.h"

using namespace topskytower;
using namespace topskytower::euroscope;

RadarScreen::RadarScreen() :
        EuroScopePlugIn::CRadarScreen(),
        m_updateFlightRegistry(false),
        m_initialized(false),
        m_airport(),
        m_userInterface(this),
        m_controllers(nullptr),
        m_disconnectedFlightsLock(),
        m_disconnectedFlights(),
        m_guiEuroscopeEventsLock(),
        m_guiEuroscopeEvents(),
        m_lastRenderingTime() { }

RadarScreen::RadarScreen(bool updateFlightRegistry) :
        EuroScopePlugIn::CRadarScreen(),
        m_updateFlightRegistry(updateFlightRegistry),
        m_initialized(false),
        m_airport(),
        m_userInterface(this),
        m_controllers(new surveillance::SectorControl()),
        m_disconnectedFlightsLock(),
        m_disconnectedFlights(),
        m_guiEuroscopeEventsLock(),
        m_guiEuroscopeEvents(),
        m_lastRenderingTime() { }

RadarScreen::~RadarScreen() {
    if (nullptr != this->m_controllers)
        delete this->m_controllers;
}

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

void RadarScreen::OnClickScreenObject(int objectType, const char* objectId, POINT pt, RECT area, int button) {
    (void)area;

    /* forward to UI manager */
    if (RadarScreen::ClickId::UserWindow == static_cast<RadarScreen::ClickId>(objectType)) {
        /* get the click point */
        Gdiplus::PointF point(static_cast<Gdiplus::REAL>(pt.x), static_cast<Gdiplus::REAL>(pt.y));

        this->m_userInterface.click(objectId, point, static_cast<UiManager::MouseButton>(button));
    }

    /* reset UI elements, if needed */
    if (RadarScreen::ClickId::UserWindow != static_cast<RadarScreen::ClickId>(objectType))
        this->m_userInterface.resetClickStates();
}

void RadarScreen::OnMoveScreenObject(int objectType, const char* objectId, POINT pt, RECT area, bool released) {
    (void)area;

    /* forward to UI manager */
    if (RadarScreen::ClickId::UserWindow == static_cast<RadarScreen::ClickId>(objectType)) {
        /* get the click point */
        Gdiplus::PointF point(static_cast<Gdiplus::REAL>(pt.x), static_cast<Gdiplus::REAL>(pt.y));

        this->m_userInterface.move(objectId, point, released);
    }
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
    if (true == this->m_updateFlightRegistry) {
        std::lock_guard guard(this->m_disconnectedFlightsLock);
        this->m_disconnectedFlights.push_back(flightPlan.GetCallsign());
    }
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

        this->m_initialized = true;
    }
}

void RadarScreen::OnRefresh(HDC hdc, int phase) {
    (void)hdc;

    if (EuroScopePlugIn::REFRESH_PHASE_BEFORE_TAGS != phase)
        return;

    Gdiplus::Graphics graphics(hdc);
    graphics.SetPageUnit(Gdiplus::UnitPixel);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    /* visualize everything of the UI manager */
    this->m_userInterface.visualize(&graphics);

    /* check if we need to initialize the system */
    this->initialize();
    if (false == this->m_initialized)
        return;

    /* remove disconnected flights */
    if (true == this->m_updateFlightRegistry) {
        this->m_disconnectedFlightsLock.lock();
        for (const auto& callsign : std::as_const(this->m_disconnectedFlights)) {
            surveillance::FlightRegistry::instance().removeFlight(callsign);
            this->m_controllers->removeFlight(callsign);
        }
        this->m_disconnectedFlights.clear();
        this->m_disconnectedFlightsLock.unlock();
    }

    /* execute one ES function event */
    if (0 != this->m_guiEuroscopeEvents.size()) {
        /* get the event and free the log to call the rest */
        this->m_guiEuroscopeEventsLock.lock();
        auto esEvent = this->m_guiEuroscopeEvents.front();
        this->m_guiEuroscopeEvents.pop_front();
        this->m_guiEuroscopeEventsLock.unlock();

        /* start the function call */
        this->StartTagFunction(esEvent.callsign.c_str(), PLUGIN_NAME, 0, esEvent.itemString.c_str(), PLUGIN_NAME,
                               esEvent.tagItemFunction, esEvent.point, esEvent.area);
    }

    auto plugin = static_cast<PlugIn*>(this->GetPlugIn());
    std::string_view positionId(plugin->ControllerMyself().GetPositionId());
    if (0 != positionId.length() && "XX" != positionId)
        this->m_controllers->setOwnSector(Converter::convert(plugin->ControllerMyself()));

    /* update the internal information of the radar targets */
    for (auto rt = plugin->RadarTargetSelectFirst(); true == rt.IsValid(); rt = plugin->RadarTargetSelectNext(rt)) {
        types::Flight flight;

        if (true == this->m_updateFlightRegistry) {
            flight = Converter::convert(rt, this->m_airport);
            surveillance::FlightRegistry::instance().updateFlight(flight);
        }
        else {
            flight = surveillance::FlightRegistry::instance().flight(rt.GetCallsign());
        }

        this->m_controllers->update(flight);
    }

    this->m_lastRenderingTime = std::chrono::system_clock::now();
}

surveillance::SectorControl& RadarScreen::sectorControl() {
    return *this->m_controllers;
}

const surveillance::SectorControl& RadarScreen::sectorControl() const {
    return *this->m_controllers;
}

void RadarScreen::registerEuroscopeEvent(RadarScreen::EuroscopeEvent&& entry) {
    std::lock_guard guard(this->m_guiEuroscopeEventsLock);
    this->m_guiEuroscopeEvents.push_back(std::move(entry));
}

const std::string& RadarScreen::airportIcao() const {
    return this->m_airport;
}

UiManager& RadarScreen::uiManager() {
    return this->m_userInterface;
}

const std::chrono::system_clock::time_point& RadarScreen::lastRenderingTime() const {
    return this->m_lastRenderingTime;
}
