/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the RADAR display of EuroScope
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <formats/AirportFileFormat.h>
#include <formats/EseFileFormat.h>
#include <helper/Exception.h>
#include <management/NotamControl.h>
#include <management/PdcControl.h>

#include "ui/elements/Text.h"

#include "Converter.h"
#include "PlugIn.h"
#include "RadarScreen.h"

using namespace topskytower;
using namespace topskytower::euroscope;
using namespace topskytower::types;

RadarScreen::RadarScreen() :
        EuroScopePlugIn::CRadarScreen(),
        m_initialized(false),
        m_sectorFileIsMissing(false),
        m_airport(),
        m_userInterface(this),
        m_flightRegistry(new system::FlightRegistry()),
        m_sectorControl(nullptr),
        m_standControl(nullptr),
        m_ariwsControl(nullptr),
        m_cmacControl(nullptr),
        m_mtcdControl(nullptr),
        m_stcdControl(nullptr),
        m_guiEuroscopeEventsLock(),
        m_guiEuroscopeEvents(),
        m_lastRenderingTime(),
        m_surveillanceVisualizationsLock(),
        m_surveillanceVisualizations(),
        m_departureRouteVisualizationsLock(),
        m_departureRouteVisualizations(),
        m_standOnScreenSelection(false),
        m_standOnScreenSelectionCallsign() { }

RadarScreen::~RadarScreen() {
    if (nullptr != this->m_ariwsControl)
        delete this->m_ariwsControl;
    if (nullptr != this->m_cmacControl)
        delete this->m_cmacControl;
    if (nullptr != this->m_mtcdControl)
        delete this->m_mtcdControl;
    if (nullptr != this->m_stcdControl)
        delete this->m_stcdControl;
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
            management::NotamControl::instance().addAirport(this->m_airport);

            auto configuration = system::ConfigurationRegistry::instance().runtimeConfiguration();
            if (configuration.windInformation.cend() == configuration.windInformation.find(this->m_airport)) {
                configuration.windInformation[this->m_airport] = types::WindData();
                system::ConfigurationRegistry::instance().setRuntimeConfiguration(configuration);
            }
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

    switch (static_cast<RadarScreen::ClickId>(objectType)) {
    case RadarScreen::ClickId::UserWindow:
    {
        /* get the click point and forward it to the UI manager */
        Gdiplus::PointF point(static_cast<Gdiplus::REAL>(pt.x), static_cast<Gdiplus::REAL>(pt.y));
        this->m_userInterface.click(objectId, point, static_cast<UiManager::MouseButton>(button));
        break;
    }
    case RadarScreen::ClickId::StandSelect:
        if (UiManager::MouseButton::Left == static_cast<UiManager::MouseButton>(button)) {
            EuroscopeEvent esEvent = {
                static_cast<int>(PlugIn::TagItemFunction::StandControlManualSelect),
                this->m_standOnScreenSelectionCallsign,
                objectId,
                pt,
                area
            };
            this->registerEuroscopeEvent(std::move(esEvent));
        }
        break;
    default:
        break;
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
    if (nullptr != this->m_stcdControl)
        this->m_stcdControl->updateFlight(flight);
}

void RadarScreen::OnFlightPlanFlightPlanDataUpdate(EuroScopePlugIn::CFlightPlan flightPlan) {
    if (false == flightPlan.GetCorrelatedRadarTarget().IsValid() || false == this->isInitialized())
        return;

    this->m_flightRegistry->updateFlight(Converter::convert(flightPlan.GetCorrelatedRadarTarget(), *this));
}

void RadarScreen::OnFlightPlanControllerAssignedDataUpdate(EuroScopePlugIn::CFlightPlan flightPlan, int type) {
    /* handle only relevant changes */
    if (EuroScopePlugIn::CTR_DATA_TYPE_TEMPORARY_ALTITUDE != type &&
        EuroScopePlugIn::CTR_DATA_TYPE_SQUAWK != type &&
        EuroScopePlugIn::CTR_DATA_TYPE_SCRATCH_PAD_STRING != type)
    {
        return;
    }

    if (false == flightPlan.GetCorrelatedRadarTarget().IsValid() || false == this->isInitialized())
        return;

    /* update the internal structures that are effected by the flight plan changes */
    auto flight = Converter::convert(flightPlan.GetCorrelatedRadarTarget(), *this);
    this->m_flightRegistry->updateFlight(flight);
    this->m_ariwsControl->updateFlight(flight);
    this->m_cmacControl->updateFlight(flight);
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
    if (nullptr != this->m_stcdControl)
        this->m_stcdControl->removeFlight(flightPlan.GetCallsign());

    surveillance::FlightPlanControl::instance().removeFlight(flightPlan.GetCallsign());
}

void RadarScreen::initialize() {
    if (true == this->m_initialized)
        return;

    if (0 == this->m_airport.length())
        return;

    auto sctFilename = this->GetPlugIn()->ControllerMyself().GetSectorFileName();
    if (true == this->m_sectorFileIsMissing) {
        static bool errorLog = false;

        if (false == errorLog) {
            this->GetPlugIn()->DisplayUserMessage("Message", "TopSky-Tower", "Unable to find the sector file", true, true, true, true, false);
            errorLog = true;
        }

        return;
    }

    /* received the correct sector filename identifier */
    if (nullptr != sctFilename && 0 != std::strlen(sctFilename)) {
        formats::EseFileFormat file;

        try {
            file = formats::EseFileFormat(sctFilename);
            if (0 == file.sectors().size() || 0 == file.runways(this->m_airport).size()) {
                this->m_sectorFileIsMissing = true;
                return;
            }
        }
        catch (const helper::Exception& ex) {
            this->GetPlugIn()->DisplayUserMessage("Message", "TopSky-Tower", ex.message().c_str(), true, true, true, true, false);
            this->m_sectorFileIsMissing = true;
            return;
        }

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

        if (nullptr != this->m_stcdControl)
            delete this->m_stcdControl;
        this->m_stcdControl = new surveillance::STCDControl(this->m_airport, center, file.runways(this->m_airport));

        this->m_initialized = true;
    }
}

Gdiplus::PointF RadarScreen::convertCoordinate(const types::Coordinate& coordinate) {
    EuroScopePlugIn::CPosition position;

    position.m_Latitude = coordinate.latitude().convert(types::degree);
    position.m_Longitude = coordinate.longitude().convert(types::degree);
    POINT pixel = this->ConvertCoordFromPositionToPixel(position);

    return Gdiplus::PointF(static_cast<float>(pixel.x), static_cast<float>(pixel.y));
}

void RadarScreen::estimateOffsets(Gdiplus::PointF& start, Gdiplus::PointF& center, Gdiplus::PointF& end,
                                  float& offsetX, float& offsetY, bool& alignRight) {
    offsetX = offsetY = 0.0f;
    alignRight = false;

    /* both flights are left of -> no vertical offset, but on the right side */
    if (center.X >= start.X && center.X >= end.X) {
        offsetX = 10.0f;
    }
    /* both flights are right -> no vertical offset, but on the left side */
    else if (center.X < start.X && center.X < end.X) {
        alignRight = true;
        offsetX = -10.0f;
    }
    /* one on the left, the other on the right */
    else if (center.X >= start.X && center.X < end.X) {
        offsetX = 10.0f;
        if (center.Y >= end.Y)
            offsetY = 10.0f;
        else
            offsetY = -30.0f;
    }
    else {
        alignRight = true;
        offsetX = -10.0f;
        if (center.Y >= start.Y)
            offsetY = 10.0f;
        else
            offsetY = -30.0f;
    }
}

void RadarScreen::drawTexts(const Gdiplus::PointF& center, float offsetX, float offsetY, bool alignRight,
                            const std::list<std::string>& lines, Gdiplus::Graphics& graphics) {
    if (0 == lines.size())
        return;

    const auto& config = system::ConfigurationRegistry::instance().systemConfiguration();
    float positionY = center.Y + offsetY;

    Text text;
    text.setGraphics(&graphics);
    text.setFontColor(Gdiplus::Color(config.uiForegroundColor[0], config.uiForegroundColor[1], config.uiForegroundColor[2]));

    for (const auto& line : std::as_const(lines)) {
        Gdiplus::PointF position(0.0f, positionY);

        text.setText(line);

        if (true == alignRight)
            position.X = offsetX + center.X - text.rectangle().Width;
        else
            position.X = offsetX + center.X;
        position.Y = positionY;

        text.setPosition(position);
        text.visualize();
        positionY += text.rectangle().Height;
    }
}

bool RadarScreen::visualizeMTCD(const std::string& callsign, Gdiplus::Graphics& graphics) {
    const auto& flight = this->m_flightRegistry->flight(callsign);
    if (false == this->m_mtcdControl->conflictsExist(flight))
        return false;

    const auto& config = system::ConfigurationRegistry::instance().systemConfiguration();
    Gdiplus::Color foreground(config.uiForegroundColor[0], config.uiForegroundColor[1], config.uiForegroundColor[2]);
    Gdiplus::Pen pen(foreground, 2.0f);
    Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 0));

    auto pixelPos = this->convertCoordinate(flight.currentPosition().coordinate());
    const auto& conflicts = this->m_mtcdControl->conflicts(flight);
    for (const auto& conflict : std::as_const(conflicts)) {
        if (false == this->m_flightRegistry->flightExists(conflict.callsign))
            continue;

        const auto& other = this->m_flightRegistry->flight(conflict.callsign);

        auto otherPos = this->convertCoordinate(other.currentPosition().coordinate());
        auto conflictPos = this->convertCoordinate(conflict.position.coordinate);

        /* estimate the optimal position for the text */
        float offsetX, offsetY;
        bool alignRight;
        RadarScreen::estimateOffsets(pixelPos, conflictPos, otherPos, offsetX, offsetY, alignRight);

        std::stringstream stream;
        stream << std::fixed << std::setprecision(1);

        /* prepare the texts */
        std::list<std::string> lines;
        stream << static_cast<int>(conflict.position.altitudeDifference.convert(types::feet)) << "ft";
        lines.push_back(stream.str());
        stream.str("");
        stream << conflict.position.horizontalSpacing.convert(types::nauticmile) << "nm";
        lines.push_back(stream.str());
        stream.str("");
        auto minutes = static_cast<int>(conflict.position.conflictIn.convert(types::second)) / 60;
        auto seconds = static_cast<int>(conflict.position.conflictIn.convert(types::second)) % 60;
        stream << minutes << "min " << seconds << "s";
        lines.push_back(stream.str());

        /* draw the lines between the flights and the conflict position */
        graphics.DrawLine(&pen, pixelPos, conflictPos);
        graphics.DrawLine(&pen, otherPos, conflictPos);

        /* draw the conflict circle */
        Gdiplus::RectF area(conflictPos.X - 3.0f, conflictPos.Y - 3.0f, 6.0f, 6.0f);
        graphics.FillEllipse(&brush, area);

        RadarScreen::drawTexts(conflictPos, offsetX, offsetY, alignRight, lines, graphics);
    }

    return 0 != conflicts.size();
}

bool RadarScreen::visualizeRoute(const std::string& callsign, Gdiplus::Graphics& graphics) {
    const auto& flight = this->m_flightRegistry->flight(callsign);

    if (false == this->m_mtcdControl->departureModelExists(flight))
        return false;

    const auto& model = this->m_mtcdControl->departureModel(flight);
    const auto& config = system::ConfigurationRegistry::instance().systemConfiguration();
    Gdiplus::Color foreground(config.uiForegroundColor[0], config.uiForegroundColor[1], config.uiForegroundColor[2]);
    Gdiplus::Pen pen(foreground, 2.0f);

    auto previousPos = this->convertCoordinate(model.waypoints()[0].position.coordinate());
    auto currentPos = this->convertCoordinate(model.waypoints()[1].position.coordinate());
    graphics.DrawLine(&pen, previousPos, currentPos);

    /* draw the complete route */
    for (std::size_t i = 1; i < model.waypoints().size() - 1; ++i) {
        currentPos = this->convertCoordinate(model.waypoints()[i].position.coordinate());
        auto nextPos = this->convertCoordinate(model.waypoints()[i + 1].position.coordinate());

        graphics.DrawLine(&pen, currentPos, nextPos);

        /* prepare the information to draw the relevant flight data */
        float offsetX, offsetY;
        bool alignRight;
        RadarScreen::estimateOffsets(previousPos, currentPos, nextPos, offsetX, offsetY, alignRight);

        /* generate the text */
        std::list<std::string> lines;
        std::stringstream stream;
        auto limit = static_cast<int>(model.waypoints()[i].position.altitude().convert(types::feet));
        limit /= 100;
        if (this->GetPlugIn()->GetTransitionAltitude() <= model.waypoints()[i].position.altitude().convert(types::feet))
            stream << "FL" << std::setw(3) << std::setfill('0') << limit;
        else
            stream << "A" << std::setw(3) << std::setfill('0') << limit;
        lines.push_back(stream.str());
        stream.str("");
        stream.clear();
        stream << static_cast<int>(model.waypoints()[i].speed.convert(types::knot)) << "kn";
        lines.push_back(stream.str());
        stream.str("");
        auto minutes = static_cast<int>(model.waypoints()[i].reachingIn.convert(types::second)) / 60;
        auto seconds = static_cast<int>(model.waypoints()[i].reachingIn.convert(types::second)) % 60;
        stream << minutes << "min " << seconds << "s";
        lines.push_back(stream.str());

        /* draw the texts */
        RadarScreen::drawTexts(currentPos, offsetX, offsetY, alignRight, lines, graphics);
    }

    return 0 != model.waypoints().size();
}

void RadarScreen::drawData(std::mutex& lock, std::list<std::pair<std::string, std::chrono::system_clock::time_point>>& data,
                           bool surveillanceData, Gdiplus::Graphics& graphics) {
    /* draw the different visualization results */
    lock.lock();
    for (auto it = data.begin(); data.end() != it;) {
        /* erase the visualization if the flight does not exist anymore */
        if (false == this->m_flightRegistry->flightExists(it->first)) {
            it = data.erase(it);
            continue;
        }

        /* check if the result is outdated */
        auto duration = static_cast<float>(std::chrono::duration_cast<std::chrono::seconds>(this->m_lastRenderingTime - it->second).count()) * types::second;
        if (duration >= system::ConfigurationRegistry::instance().systemConfiguration().surveillanceVisualizationDuration) {
            it = data.erase(it);
            continue;
        }

        if (true == surveillanceData) {
            /* draw the MTCD conflicts */
            if (true == this->visualizeMTCD(it->first, graphics))
                ++it;
            /* no more conflicts available */
            else
                it = data.erase(it);
        }
        else {
            if (true == this->visualizeRoute(it->first, graphics))
                ++it;
            else
                it = data.erase(it);
        }
    }
    lock.unlock();
}

void RadarScreen::drawNoTransgressionZones(Gdiplus::Graphics& graphics) {
    if (nullptr == this->m_stcdControl)
        return;

    const auto& config = system::ConfigurationRegistry::instance().systemConfiguration();
    Gdiplus::Pen pen(Gdiplus::Color(config.uiNtzColor[0], config.uiNtzColor[1], config.uiNtzColor[2]), 1.0f);

    /* iterate over every NTZ */
    const auto& ntzs = this->m_stcdControl->noTransgressionZones();
    for (const auto& ntz : std::as_const(ntzs)) {
        /* skip an empty NTZ */
        if (0 == ntz.edges().size())
            continue;

        /* draw the lines over all edges */
        auto prev = this->convertCoordinate(ntz.edges().front());
        auto current = this->convertCoordinate(ntz.edges().back());
        graphics.DrawLine(&pen, prev, current);
        auto it = ntz.edges().cbegin();
        std::advance(it, 1);
        for (; ntz.edges().cend() != it; ++it) {
            current = this->convertCoordinate(*it);
            graphics.DrawLine(&pen, prev, current);
            prev = current;
        }
    }
}

void RadarScreen::OnRefresh(HDC hdc, int phase) {
    (void)hdc;

    if (EuroScopePlugIn::REFRESH_PHASE_AFTER_TAGS != phase)
        return;

    this->m_lastRenderingTime = std::chrono::system_clock::now();
    Gdiplus::Graphics graphics(hdc);
    graphics.SetPageUnit(Gdiplus::UnitPixel);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    this->drawData(this->m_surveillanceVisualizationsLock, this->m_surveillanceVisualizations, true, graphics);
    this->drawData(this->m_departureRouteVisualizationsLock, this->m_departureRouteVisualizations, false, graphics);
    this->drawNoTransgressionZones(graphics);

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

    /* update all states of the flights */
    for (auto rt = plugin->RadarTargetSelectFirst(); true == rt.IsValid(); rt = plugin->RadarTargetSelectNext(rt))
        this->m_flightRegistry->updateFlight(Converter::convert(rt, *this));

    /* add the UI elements for the ground menu */
    if (nullptr != this->m_standControl && true == this->m_standOnScreenSelection) {
        auto stands = this->m_standControl->allPossibleAndAvailableStands(this->m_flightRegistry->flight(this->m_standOnScreenSelectionCallsign));
        auto radarArea = this->GetRadarArea();
        Gdiplus::Color color(
            system::ConfigurationRegistry::instance().systemConfiguration().uiScreenClickColor[0],
            system::ConfigurationRegistry::instance().systemConfiguration().uiScreenClickColor[1],
            system::ConfigurationRegistry::instance().systemConfiguration().uiScreenClickColor[2]
        );
        Gdiplus::Pen pen(color, 1.0f);

        for (const auto& stand : std::as_const(stands)) {
            const auto& standData = this->m_standControl->stand(stand);
            const auto pixelPos = this->convertCoordinate(standData.position);

            /* the stand is inside the radar area */
            if (pixelPos.X >= radarArea.left && pixelPos.X < radarArea.right && pixelPos.Y >= radarArea.top && pixelPos.Y < radarArea.bottom) {
                RECT rect = {
                    static_cast<int>(pixelPos.X) - 5, static_cast<int>(pixelPos.Y) - 5,
                    static_cast<int>(pixelPos.X) + 5, static_cast<int>(pixelPos.Y) + 5
                };
                this->AddScreenObject(static_cast<int>(RadarScreen::ClickId::StandSelect), standData.name.c_str(), rect, false, "");

                Gdiplus::Rect gdiRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
                graphics.DrawEllipse(&pen, gdiRect);
            }
        }
    }
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

surveillance::STCDControl& RadarScreen::stcdControl() const {
    return *this->m_stcdControl;
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

void RadarScreen::activateSurveillanceVisualization(const std::string& callsign) {
    std::lock_guard guard(this->m_surveillanceVisualizationsLock);

    auto stamp = std::chrono::system_clock::now();

    for (auto& entry : this->m_surveillanceVisualizations) {
        if (entry.first == callsign) {
            entry.second = stamp;
            return;
        }
    }

    this->m_surveillanceVisualizations.push_back(std::make_pair(callsign, stamp));
}

void RadarScreen::activateDepartureRouteVisualization(const std::string& callsign) {
    std::lock_guard guard(this->m_departureRouteVisualizationsLock);

    auto stamp = std::chrono::system_clock::now();

    for (auto& entry : this->m_departureRouteVisualizations) {
        if (entry.first == callsign) {
            entry.second = stamp;
            return;
        }
    }

    this->m_departureRouteVisualizations.push_back(std::make_pair(callsign, stamp));
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
         * Assumption:
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

void RadarScreen::activateStandOnScreenSelection(bool activate, const std::string& callsign) {
    this->m_standOnScreenSelection = activate;
    this->m_standOnScreenSelectionCallsign = callsign;
}
