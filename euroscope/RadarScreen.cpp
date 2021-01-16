/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the RADAR display of EuroScope
 */

#include "stdafx.h"

#include <formats/AirportFileFormat.h>
#include <formats/EseFileFormat.h>
#include <management/PdcControl.h>

#include "Converter.h"
#include "PlugIn.h"
#include "RadarScreen.h"

using namespace topskytower;
using namespace topskytower::euroscope;
using namespace topskytower::types;

RadarScreen::RadarScreen() :
        EuroScopePlugIn::CRadarScreen(),
        m_initialized(false),
        m_airport(),
        m_userInterface(this),
        m_flightRegistry(new system::FlightRegistry()),
        m_sectorControl(nullptr),
        m_standControl(nullptr),
        m_ariwsControl(nullptr),
        m_cmacControl(nullptr),
        m_mtcdControl(nullptr),
        m_guiEuroscopeEventsLock(),
        m_guiEuroscopeEvents(),
        m_lastRenderingTime() { }

RadarScreen::~RadarScreen() {
    if (nullptr != this->m_ariwsControl)
        delete this->m_ariwsControl;
    if (nullptr != this->m_cmacControl)
        delete this->m_cmacControl;
    if (nullptr != this->m_mtcdControl)
        delete this->m_mtcdControl;
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
        if (nullptr != value) {
            this->m_airport = value;
            system::ConfigurationRegistry::instance().runtimeConfiguration().windInformation[this->m_airport] = types::WindData();
        }
        else {
            this->GetPlugIn()->DisplayUserMessage("Message", "TopSky-Tower", "No airport in the ASR file defined",
                                                  true, true, false, false, false);
        }
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
    if (nullptr != this->m_ariwsControl)
        this->m_ariwsControl->updateFlight(flight);
    if (nullptr != this->m_cmacControl)
        this->m_cmacControl->updateFlight(flight);
    if (nullptr != this->m_mtcdControl)
        this->m_mtcdControl->updateFlight(flight);
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
    if (nullptr != this->m_ariwsControl)
        this->m_ariwsControl->removeFlight(flightPlan.GetCallsign());
    if (nullptr != this->m_cmacControl)
        this->m_cmacControl->removeFlight(flightPlan.GetCallsign());
    if (nullptr != this->m_mtcdControl)
        this->m_mtcdControl->removeFlight(flightPlan.GetCallsign());

    surveillance::FlightPlanControl::instance().removeFlight(flightPlan.GetCallsign());
}

void RadarScreen::initialize() {
    if (true == this->m_initialized)
        return;

    if (0 == this->m_airport.length())
        return;

    auto sctFilename = this->GetPlugIn()->ControllerMyself().GetSectorFileName();

    /* received the correct sector filename identifier */
    if (nullptr != sctFilename && 0 != std::strlen(sctFilename)) {
        formats::EseFileFormat file(sctFilename);
        if (0 == file.sectors().size())
            return;

        if (nullptr != this->m_sectorControl)
            delete this->m_sectorControl;
        this->m_sectorControl = new management::SectorControl(this->m_airport, file.sectors());

        /* find the center of the airport */
        types::Coordinate center;
        for (const auto& sector : std::as_const(file.sectors())) {
            if (sector.controllerInfo().prefix() == this->m_airport) {
                center = sector.controllerInfo().centerPoint();
                break;
            }
        }

        if (nullptr != this->m_standControl)
            delete this->m_standControl;
        this->m_standControl = new management::StandControl(this->m_airport, center);

        if (nullptr != this->m_ariwsControl)
            delete this->m_ariwsControl;
        this->m_ariwsControl = new surveillance::ARIWSControl(this->m_airport, center);

        if (nullptr != this->m_cmacControl)
            delete this->m_cmacControl;
        this->m_cmacControl = new surveillance::CMACControl();

        if (nullptr != this->m_mtcdControl)
            delete this->m_mtcdControl;
        this->m_mtcdControl = new surveillance::MTCDControl(this->m_airport, center);
        this->m_mtcdControl->registerSidExtraction(this, &RadarScreen::extractPredictedSID);

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

management::SectorControl& RadarScreen::sectorControl() {
    return *this->m_sectorControl;
}

system::FlightRegistry& RadarScreen::flightRegistry() const {
    return *this->m_flightRegistry;
}

management::StandControl& RadarScreen::standControl() const {
    return *this->m_standControl;
}

surveillance::ARIWSControl& RadarScreen::ariwsControl() const {
    return *this->m_ariwsControl;
}

surveillance::CMACControl& RadarScreen::cmacControl() const {
    return *this->m_cmacControl;
}

surveillance::MTCDControl& RadarScreen::mtcdControl() const {
    return *this->m_mtcdControl;
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

std::vector<types::Coordinate> RadarScreen::extractPredictedSID(const std::string& callsign) {
    auto flightPlan = this->GetPlugIn()->FlightPlanSelect(callsign.c_str());
    std::vector<types::Coordinate> retval;

    if (false == flightPlan.IsValid())
        return retval;

    /* search the SID exit point */
    const auto& flight = this->flightRegistry().flight(callsign);
    auto route = flight.flightPlan().route().waypoints();
    types::Coordinate sidExit;
    bool foundExit = false;
    for (const auto& waypoint : std::as_const(route)) {
        if (std::string::npos != flight.flightPlan().departureRoute().find(waypoint.name())) {
            sidExit = waypoint.position();
            foundExit = true;
            break;
        }
    }

    /* fallback to allow a maximum distance to filter the route */
    types::Length maxDistance = 1000.0_nm;
    if (false == foundExit)
        maxDistance = 50.0_nm;

    types::Length lastDistance = 1000 * types::nauticmile;
    retval.reserve(flightPlan.GetPositionPredictions().GetPointsNumber() - 1);
    for (int i = 1; i < flightPlan.GetPositionPredictions().GetPointsNumber(); ++i) {
        auto position = Converter::convert(flightPlan.GetPositionPredictions().GetPosition(i));

        auto distance = sidExit.distanceTo(position);

        /*
         * Assumtion:
         *  - The distance between the current position and the sidExit decreases until a certain point
         *    where we follow the rest of the route
         */
        if (true == foundExit) {
            if (lastDistance < 10 * types::nauticmile) {
                /* the distance increases again -> found the closest point in the last iteration */
                if (distance > lastDistance) {
                    retval[retval.size() - 1] = sidExit;
                    break;
                }
            }
        }
        else {
            maxDistance -= distance;
            if (0.0_m > maxDistance)
                break;
        }

        retval.push_back(position);
        lastDistance = distance;
    }

    /* check if we found the complete SID route */
    if (true == foundExit && 0 != retval.size()) {
        if (10_nm < sidExit.distanceTo(retval.back()))
            return std::vector<types::Coordinate>();
    }

    return std::move(retval);
}
