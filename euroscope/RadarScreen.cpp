/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the RADAR display of EuroScope
 */

#include "stdafx.h"

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
        m_initialized(false),
        m_airport(),
        m_userInterface(this),
        m_flightRegistry(new system::FlightRegistry()),
        m_sectorControl(nullptr),
        m_standControl(nullptr),
        m_guiEuroscopeEventsLock(),
        m_guiEuroscopeEvents(),
        m_lastRenderingTime() { }

RadarScreen::~RadarScreen() {
    if (nullptr != this->m_sectorControl)
        delete this->m_sectorControl;
    if (nullptr != this->m_standControl)
        delete this->m_standControl;
    if (nullptr != this->m_flightRegistry)
        delete this->m_flightRegistry;
}

void RadarScreen::OnAsrContentLoaded(bool loaded) {
    if (true == loaded) {
        auto value = this->GetDataFromAsr("Airport");
        if (nullptr != value)
            this->m_airport = value;
        else
            this->GetPlugIn()->DisplayUserMessage("Message", "TopSky-Tower", "No airport in the ASR file defined",
                                                  true, true, false, false, false);
    }
}

void RadarScreen::OnAsrContentToBeClosed() {
    static_cast<PlugIn*>(this->GetPlugIn())->removeRadarScreen(this);
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
    if (nullptr != this->m_sectorControl && true == controller.IsValid() && true == controller.IsController()) {
        std::string_view view(controller.GetPositionId());
        if ("" != view && "XX" != view)
            this->m_sectorControl->controllerUpdate(Converter::convert(controller));
    }
}

void RadarScreen::OnControllerDisconnect(EuroScopePlugIn::CController controller) {
    if (nullptr != this->m_sectorControl && true == controller.IsValid() && true == controller.IsController()) {
        std::string_view view(controller.GetPositionId());
        if ("" != view && "XX" != view)
            this->m_sectorControl->controllerOffline(Converter::convert(controller));
    }
}

void RadarScreen::OnRadarTargetPositionUpdate(EuroScopePlugIn::CRadarTarget radarTarget) {
    auto flight = Converter::convert(radarTarget, *this);
    this->m_flightRegistry->updateFlight(flight);

    if (nullptr != this->m_sectorControl)
        this->m_sectorControl->updateFlight(flight);

    if (nullptr != this->m_standControl)
        this->m_standControl->updateFlight(flight);
}

void RadarScreen::OnFlightPlanFlightPlanDataUpdate(EuroScopePlugIn::CFlightPlan flightPlan) {
    if (false == flightPlan.GetCorrelatedRadarTarget().IsValid())
        return;

    auto flight = Converter::convert(flightPlan.GetCorrelatedRadarTarget(), *this);
    this->m_flightRegistry->updateFlight(flight);
}

void RadarScreen::OnFlightPlanControllerAssignedDataUpdate(EuroScopePlugIn::CFlightPlan flightPlan, int type) {
    /* handle only relevant changes */
    if (EuroScopePlugIn::CTR_DATA_TYPE_TEMPORARY_ALTITUDE != type &&
        EuroScopePlugIn::CTR_DATA_TYPE_SQUAWK != type &&
        EuroScopePlugIn::CTR_DATA_TYPE_SCRATCH_PAD_STRING != type)
    {
        return;
    }

    if (false == flightPlan.GetCorrelatedRadarTarget().IsValid())
        return;

    auto flight = Converter::convert(flightPlan.GetCorrelatedRadarTarget(), *this);
    this->m_flightRegistry->updateFlight(flight);
}

void RadarScreen::OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan flightPlan) {
    this->m_flightRegistry->removeFlight(flightPlan.GetCallsign());

    if (nullptr != this->m_standControl)
        this->m_standControl->removeFlight(flightPlan.GetCallsign());
    if (nullptr != this->m_sectorControl)
        this->m_sectorControl->removeFlight(flightPlan.GetCallsign());
}

void RadarScreen::initialize() {
    if (0 == this->m_airport.length())
        return;

    if (true == this->m_initialized)
        return;

    auto sctFilename = this->GetPlugIn()->ControllerMyself().GetSectorFileName();

    /* received the correct sector filename identifier */
    if (nullptr != sctFilename && 0 != std::strlen(sctFilename)) {
        formats::EseFileFormat file(sctFilename);
        if (0 == file.sectors().size())
            return;

        if (nullptr != this->m_sectorControl)
            delete this->m_sectorControl;
        this->m_sectorControl = new surveillance::SectorControl(this->m_airport, file.sectors());

        if (nullptr != this->m_standControl)
            delete this->m_standControl;
        this->m_standControl = new surveillance::StandControl(this->m_airport);

        this->m_initialized = true;
    }
}

void RadarScreen::OnRefresh(HDC hdc, int phase) {
    (void)hdc;

    if (EuroScopePlugIn::REFRESH_PHASE_AFTER_TAGS != phase)
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

    auto plugin = static_cast<PlugIn*>(this->GetPlugIn());

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

    std::string_view positionId(plugin->ControllerMyself().GetPositionId());
    if (0 != positionId.length() && "XX" != positionId)
        this->m_sectorControl->setOwnSector(Converter::convert(plugin->ControllerMyself()));

    this->m_lastRenderingTime = std::chrono::system_clock::now();
}

surveillance::SectorControl& RadarScreen::sectorControl() {
    return *this->m_sectorControl;
}

system::FlightRegistry& RadarScreen::flightRegistry() const {
    return *this->m_flightRegistry;
}

surveillance::StandControl& RadarScreen::standControl() const {
    return *this->m_standControl;
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

bool RadarScreen::isInitialized() const {
    return this->m_initialized;
}

const std::chrono::system_clock::time_point& RadarScreen::lastRenderingTime() const {
    return this->m_lastRenderingTime;
}
