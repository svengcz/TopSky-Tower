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
#include <helper/String.h>
#include <management/NotamControl.h>
#include <management/PdcControl.h>
#include <surveillance/RadioControl.h>

#include "ui/ConfigurationErrorWindow.h"
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
        m_elevation(),
        m_runways(),
        m_userInterface(new UiManager(this)),
        m_sectorControl(nullptr),
        m_standControl(nullptr),
        m_departureControl(nullptr),
        m_ariwsControl(nullptr),
        m_cmacControl(nullptr),
        m_mtcdControl(nullptr),
        m_stcdControl(nullptr),
        m_guiEuroscopeEventsLock(),
        m_guiEuroscopeEvents(),
        m_lastRenderingTime(),
        m_mediumTermConflictVisualizationsLock(),
        m_mediumTermConflictVisualizations(),
        m_departureRouteVisualizationsLock(),
        m_departureRouteVisualizations(),
        m_shortTermConflictVisualizationsLock(),
        m_shortTermConflictVisualizations(),
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
    if (nullptr != this->m_departureControl)
        delete this->m_departureControl;
    if (nullptr != this->m_sectorControl)
        delete this->m_sectorControl;
    if (nullptr != this->m_standControl)
        delete this->m_standControl;
    if (nullptr != this->m_userInterface)
        delete this->m_userInterface;
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

        value = this->GetDataFromAsr("Elevation");
        if (nullptr != value) {
            this->m_elevation = static_cast<float>(std::atof(value)) * types::feet;
        }
        else {
            this->GetPlugIn()->DisplayUserMessage("Message", "TopSky-Tower", "No elevation in the ASR file defined",
                                                  true, true, false, false, false);
        }

        bool hideWindows = false;
        value = this->GetDataFromAsr("HideWindows");
        if (nullptr != value)
            hideWindows = 0 != std::strlen(value) && '1' == value[0];
        this->m_userInterface->hideWindows(hideWindows);

        if (true == system::ConfigurationRegistry::instance().errorFound()) {
            auto viewer = new ConfigurationErrorWindow(this);
            viewer->setActive(true);
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
        if (nullptr != this->m_userInterface) {
            Gdiplus::PointF point(static_cast<Gdiplus::REAL>(pt.x), static_cast<Gdiplus::REAL>(pt.y));
            this->m_userInterface->click(objectId, point, static_cast<UiManager::MouseButton>(button));
        }
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
    if (RadarScreen::ClickId::UserWindow != static_cast<RadarScreen::ClickId>(objectType) && nullptr != this->m_userInterface)
        this->m_userInterface->resetClickStates();
}

void RadarScreen::OnMoveScreenObject(int objectType, const char* objectId, POINT pt, RECT area, bool released) {
    (void)area;

    /* forward to UI manager */
    if (RadarScreen::ClickId::UserWindow == static_cast<RadarScreen::ClickId>(objectType) && nullptr != this->m_userInterface) {
        /* get the click point */
        Gdiplus::PointF point(static_cast<Gdiplus::REAL>(pt.x), static_cast<Gdiplus::REAL>(pt.y));

        this->m_userInterface->move(objectId, point, released);
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
    if (false == this->isInitialized())
        return;

    if (false == radarTarget.IsValid() || false == system::FlightRegistry::instance().flightExists(radarTarget.GetCallsign()))
        return;

    const auto& flight = system::FlightRegistry::instance().flight(radarTarget.GetCallsign());
    auto type = this->identifyType(flight);

    this->m_sectorControl->updateFlight(flight, type);
    this->m_standControl->updateFlight(flight, type);
    this->m_departureControl->updateFlight(flight, type);
    this->m_mtcdControl->updateFlight(flight, type);
    this->m_stcdControl->updateFlight(flight, type);
    this->m_ariwsControl->updateFlight(flight, type);
    this->m_cmacControl->updateFlight(flight, type);
}

void RadarScreen::OnFlightPlanControllerAssignedDataUpdate(EuroScopePlugIn::CFlightPlan flightPlan, int type) {
    if (false == this->isInitialized())
        return;

    /* handle only relevant changes */
    if (EuroScopePlugIn::CTR_DATA_TYPE_TEMPORARY_ALTITUDE != type &&
        EuroScopePlugIn::CTR_DATA_TYPE_SQUAWK != type &&
        EuroScopePlugIn::CTR_DATA_TYPE_SCRATCH_PAD_STRING != type)
    {
        return;
    }

    auto callsign = flightPlan.GetCallsign();
    if (false == flightPlan.GetCorrelatedRadarTarget().IsValid() || false == system::FlightRegistry::instance().flightExists(callsign))
        return;

    /* update the internal structures that are effected by the flight plan changes */
    const auto& flight = system::FlightRegistry::instance().flight(callsign);
    auto flightType = this->identifyType(flight);

    this->m_departureControl->updateFlight(flight, flightType);
    this->m_mtcdControl->updateFlight(flight, flightType);
    this->m_stcdControl->updateFlight(flight, flightType);
    this->m_ariwsControl->updateFlight(flight, flightType);
    this->m_cmacControl->updateFlight(flight, flightType);

    /* update the stand if needed */
    std::string stand = flightPlan.GetControllerAssignedData().GetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand));
    if (0 != stand.length()) {
        auto split = helper::String::splitString(stand, "/");
        if (3 == split.size() && this->m_standControl->standExists(split[1]) && this->m_standControl->stand(flight) != split[1])
            this->m_standControl->assignManually(flight, flightType, split[1]);
    }
}

void RadarScreen::OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan flightPlan) {
    if (false == this->isInitialized())
        return;

    std::string callsign = flightPlan.GetCallsign();

    this->m_standControl->removeFlight(callsign);
    this->m_sectorControl->removeFlight(callsign);
    this->m_departureControl->removeFlight(callsign);
    this->m_ariwsControl->removeFlight(callsign);
    this->m_cmacControl->removeFlight(callsign);
    this->m_mtcdControl->removeFlight(callsign);
    this->m_stcdControl->removeFlight(callsign);
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
            if (false == file.parse(sctFilename)) {
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
        this->m_cmacControl = new surveillance::CMACControl(this->m_airport, center);

        if (nullptr != this->m_departureControl)
            delete this->m_departureControl;
        this->m_departureControl = new management::DepartureSequenceControl(this->m_airport, center);

        if (nullptr != this->m_mtcdControl)
            delete this->m_mtcdControl;
        this->m_mtcdControl = new surveillance::MTCDControl(center, this->m_departureControl);
        this->m_mtcdControl->registerSidExtraction(this, &RadarScreen::extractPredictedSID);

        if (nullptr != this->m_stcdControl)
            delete this->m_stcdControl;
        this->m_stcdControl = new surveillance::STCDControl(this->m_airport, this->m_elevation, center,
                                                            file.runways(this->m_airport), this->m_departureControl);

        this->m_runways = file.runways(this->m_airport);

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

bool RadarScreen::visualizeMTCD(const types::Flight& flight, Gdiplus::Graphics& graphics) {
    if (false == this->m_mtcdControl->conflictsExist(flight))
        return false;

    const auto& config = system::ConfigurationRegistry::instance().systemConfiguration();
    Gdiplus::Color foreground(config.uiForegroundColor[0], config.uiForegroundColor[1], config.uiForegroundColor[2]);
    Gdiplus::Pen pen(foreground, 2.0f);
    Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 0));

    auto pixelPos = this->convertCoordinate(flight.currentPosition().coordinate());
    const auto& conflicts = this->m_mtcdControl->conflicts(flight);
    for (const auto& conflict : std::as_const(conflicts)) {
        if (false == system::FlightRegistry::instance().flightExists(conflict.callsign))
            continue;

        const auto& other = system::FlightRegistry::instance().flight(conflict.callsign);

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

bool RadarScreen::visualizeRoute(const types::Flight& flight, Gdiplus::Graphics& graphics) {
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

bool RadarScreen::visualizeSTCD(const types::Flight& flight, Gdiplus::Graphics& graphics) {
    if (true == this->m_stcdControl->separationLoss(flight)) {
        auto minSeparation = this->m_stcdControl->minSeparation(flight);
        Gdiplus::Pen stcdPen(Gdiplus::Color(255, 0, 0), 1.0f);

        /* calculate the required coordinates to define the rectangle for the ellipse */
        auto topCoordinate = flight.currentPosition().coordinate().projection(0.0_deg, minSeparation);
        auto top = this->convertCoordinate(topCoordinate);
        auto leftCoordinate = flight.currentPosition().coordinate().projection(270.0_deg, minSeparation);
        auto left = this->convertCoordinate(leftCoordinate);
        auto center = this->convertCoordinate(flight.currentPosition().coordinate());

        /* calculate the rectangle */
        Gdiplus::RectF rect(left.X, top.Y, 2.0f * (center.X - left.X), 2.0f * (center.Y - top.Y));
        graphics.DrawEllipse(&stcdPen, rect);

        return true;
    }

    return false;
}

void RadarScreen::drawData(std::mutex& lock, std::list<std::pair<std::string, types::Time>>& data,
                           bool surveillanceData, Gdiplus::Graphics& graphics) {
    auto now = std::chrono::system_clock::now();
    auto delta = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(now - this->m_lastRenderingTime).count()) * types::millisecond;

    /* draw the different visualization results */
    lock.lock();
    for (auto it = data.begin(); data.end() != it;) {
        /* erase the visualization if the flight does not exist anymore */
        if (false == system::FlightRegistry::instance().flightExists(it->first)) {
            it = data.erase(it);
            continue;
        }

        const auto& flight = system::FlightRegistry::instance().flight(it->first);

        /* check if the result is outdated */
        if (0.0_s <= it->second) {
            it->second -= delta;
            if (0.0_s > it->second) {
                it = data.erase(it);
                continue;
            }
        }

        if (true == surveillanceData) {
            bool visualized = false;

            /* draw the STCD conflicts */
            if (true == this->m_stcdControl->separationLoss(flight))
                visualized = this->visualizeSTCD(flight, graphics);
            /* draw the MTCD conflicts */
            else
                visualized = this->visualizeMTCD(flight, graphics);

            if (true == visualized)
                ++it;
            else
                it = data.erase(it);
        }
        else {
            if (true == this->visualizeRoute(flight, graphics))
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

void RadarScreen::drawTransmittingFlights(Gdiplus::Graphics& graphics) {
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().rdfActive)
        return;

    auto callsigns = surveillance::RadioControl::instance().transmittingFlights();

    const std::uint8_t* colorValues;
    if (2 > callsigns.size())
        colorValues = system::ConfigurationRegistry::instance().systemConfiguration().rdfNonConflictColor;
    else
        colorValues = system::ConfigurationRegistry::instance().systemConfiguration().rdfConflictColor;

    /* prepare generic data structures */
    auto area = this->GetRadarArea();
    Gdiplus::PointF center((area.right - area.left) * 0.5f, (area.bottom - area.top) * 0.5f);
    Gdiplus::Pen pen(Gdiplus::Color(colorValues[0], colorValues[1], colorValues[2]), 1.0f);
    Gdiplus::RectF transmissionRect;
    transmissionRect.Width = system::ConfigurationRegistry::instance().systemConfiguration().rdfRadius * 2.0f;
    transmissionRect.Height = system::ConfigurationRegistry::instance().systemConfiguration().rdfRadius * 2.0f;

    /* draw all transmitting flights */
    for (const auto& callsign : std::as_const(callsigns)) {
        if (false == system::FlightRegistry::instance().flightExists(callsign))
            continue;

        const auto& flight = system::FlightRegistry::instance().flight(callsign);
        auto pxPos = this->convertCoordinate(flight.currentPosition().coordinate());

        /* draw the circle */
        if (pxPos.X >= area.left && pxPos.Y >= area.top && pxPos.X < area.right && pxPos.Y < area.bottom) {
            transmissionRect.X = pxPos.X - system::ConfigurationRegistry::instance().systemConfiguration().rdfRadius;
            transmissionRect.Y = pxPos.Y - system::ConfigurationRegistry::instance().systemConfiguration().rdfRadius;
            graphics.DrawEllipse(&pen, transmissionRect);
        }
        /* draw the line */
        else {
            graphics.DrawLine(&pen, center, pxPos);
        }
    }
}

void RadarScreen::drawRunwayOverlay(const types::Runway& runway, Gdiplus::Graphics& graphics) {
    auto& config = system::ConfigurationRegistry::instance().systemConfiguration();

    Gdiplus::Color color(config.uiNotamDeactivationColor[0], config.uiNotamDeactivationColor[1], config.uiNotamDeactivationColor[2]);
    Gdiplus::PointF corners[4];

    corners[0] = this->convertCoordinate(runway.start().projection(runway.heading() + 90.0_deg, 50.0_m));
    corners[1] = this->convertCoordinate(runway.start().projection(runway.heading() - 90.0_deg, 50.0_m));
    corners[2] = this->convertCoordinate(runway.end().projection(runway.heading() - 90.0_deg, 50.0_m));
    corners[3] = this->convertCoordinate(runway.end().projection(runway.heading() + 90.0_deg, 50.0_m));

    Gdiplus::SolidBrush brush(color);
    graphics.FillPolygon(&brush, corners, 4);
}

void RadarScreen::drawDeactivatedRunways(Gdiplus::Graphics& graphics) {
    if (0 == this->m_runways.size())
        return;

    /* iterate over all NOTAMs */
    auto notams = management::NotamControl::instance().notams(this->m_airport, management::NotamCategory::Runway);
    for (const auto& notam : std::as_const(notams)) {
        if (management::NotamInterpreterState::Success == notam->interpreterState && true == notam->isActive()) {
            auto& runways = static_cast<management::RunwayNotam*>(notam.get())->sections;

            /* find the correct runways */
            for (std::size_t i = 0; i < runways.size(); i += 2) {
                for (const auto& runway : std::as_const(this->m_runways)) {
                    if (runway.name() == runways[i]) {
                        this->drawRunwayOverlay(runway, graphics);
                        break;
                    }
                }
            }
        }
    }
}

void RadarScreen::drawStandOverlay(const types::Stand& stand, Gdiplus::Graphics& graphics) {
    if (0 != stand.name.length()) {
        auto& config = system::ConfigurationRegistry::instance().systemConfiguration();
        Gdiplus::Color color(config.uiNotamDeactivationColor[0], config.uiNotamDeactivationColor[1], config.uiNotamDeactivationColor[2]);
        Gdiplus::SolidBrush brush(color);

        /* calculate the rectangle for the ellipse */
        auto topCenter = this->convertCoordinate(stand.position.projection(360.0_deg, stand.assignmentRadius));
        auto center = this->convertCoordinate(stand.position);
        float radius = center.Y - topCenter.Y;
        Gdiplus::PointF topLeft(center.X - radius, center.Y - radius);
        Gdiplus::RectF rect(topLeft, Gdiplus::SizeF(radius * 2.0f, radius * 2.0f));

        graphics.FillEllipse(&brush, rect);
    }
}

void RadarScreen::drawDeactivatedStands(Gdiplus::Graphics& graphics) {
    if (nullptr == this->m_standControl)
        return;

    /* iterate over the NOTAMs */
    auto notams = management::NotamControl::instance().notams(this->m_airport, management::NotamCategory::Stands);
    for (const auto& notam : std::as_const(notams)) {
        /* process active NOTAM */
        if (management::NotamInterpreterState::Success == notam->interpreterState && true == notam->isActive()) {
            auto& stands = static_cast<management::StandNotam*>(notam.get())->sections;

            /* find the stand and draw the area */
            for (const auto& name : std::as_const(stands)) {
                auto& stand = this->m_standControl->stand(name);
                this->drawStandOverlay(stand, graphics);
            }
        }
    }
}

void RadarScreen::OnRefresh(HDC hdc, int phase) {
    (void)hdc;

    if (EuroScopePlugIn::REFRESH_PHASE_AFTER_TAGS != phase)
        return;

    Gdiplus::Graphics graphics(hdc);
    graphics.SetPageUnit(Gdiplus::UnitPixel);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    this->drawData(this->m_mediumTermConflictVisualizationsLock, this->m_mediumTermConflictVisualizations, true, graphics);
    this->drawData(this->m_shortTermConflictVisualizationsLock, this->m_shortTermConflictVisualizations, true, graphics);
    this->drawData(this->m_departureRouteVisualizationsLock, this->m_departureRouteVisualizations, false, graphics);
    this->drawNoTransgressionZones(graphics);
    this->drawDeactivatedRunways(graphics);
    this->drawDeactivatedStands(graphics);
    this->drawTransmittingFlights(graphics);

    this->m_lastRenderingTime = std::chrono::system_clock::now();

    /* visualize everything of the UI manager */
    if (nullptr != this->m_userInterface)
        this->m_userInterface->visualize(&graphics);

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

    /* add the UI elements for the ground menu */
    if (nullptr != this->m_standControl && true == this->m_standOnScreenSelection) {
        auto stands = this->m_standControl->allPossibleAndAvailableStands(system::FlightRegistry::instance().flight(this->m_standOnScreenSelectionCallsign));
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

management::StandControl& RadarScreen::standControl() const {
    return *this->m_standControl;
}

surveillance::ARIWSControl& RadarScreen::ariwsControl() const {
    return *this->m_ariwsControl;
}

management::DepartureSequenceControl& RadarScreen::departureSequenceControl() const {
    return *this->m_departureControl;
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
    return *this->m_userInterface;
}

bool RadarScreen::isInitialized() const {
    return this->m_initialized;
}

const std::chrono::system_clock::time_point& RadarScreen::lastRenderingTime() const {
    return this->m_lastRenderingTime;
}

void RadarScreen::activateSurveillanceVisualization(const std::string& callsign) {
    auto duration = system::ConfigurationRegistry::instance().systemConfiguration().surveillanceVisualizationDuration;
    const auto& flight = system::FlightRegistry::instance().flight(callsign);

    if (true == this->m_stcdControl->separationLoss(flight)) {
        std::lock_guard guard(this->m_shortTermConflictVisualizationsLock);
        for (auto& entry : this->m_shortTermConflictVisualizations) {
            if (entry.first == callsign) {
                entry.second = duration;
                return;
            }
        }
        this->m_shortTermConflictVisualizations.push_back(std::make_pair(callsign, duration));
    }
    else {
        std::lock_guard guard(this->m_mediumTermConflictVisualizationsLock);
        for (auto& entry : this->m_mediumTermConflictVisualizations) {
            if (entry.first == callsign) {
                entry.second = duration;
                return;
            }
        }
        this->m_mediumTermConflictVisualizations.push_back(std::make_pair(callsign, duration));
    }
}

void RadarScreen::activateDepartureRouteVisualization(const std::string& callsign, const types::Time& visualizationDuration) {
    std::lock_guard guard(this->m_departureRouteVisualizationsLock);

    for (auto it = this->m_departureRouteVisualizations.begin(); this->m_departureRouteVisualizations.end() != it; ++it) {
        if (it->first == callsign) {
            if (0.0_s > visualizationDuration)
                this->m_departureRouteVisualizations.erase(it);
            else
                it->second = visualizationDuration;
            return;
        }
    }

    this->m_departureRouteVisualizations.push_back(std::make_pair(callsign, visualizationDuration));
}

std::vector<types::Coordinate> RadarScreen::extractPredictedSID(const std::string& callsign) {
    auto flightPlan = this->GetPlugIn()->FlightPlanSelect(callsign.c_str());
    std::vector<types::Coordinate> retval;

    if (false == flightPlan.IsValid())
        return retval;

    /* search the SID exit point */
    const auto& flight = system::FlightRegistry::instance().flight(callsign);
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

    return retval;
}

void RadarScreen::activateStandOnScreenSelection(bool activate, const std::string& callsign) {
    this->m_standOnScreenSelection = activate;
    this->m_standOnScreenSelectionCallsign = callsign;
}

types::Flight::Type RadarScreen::identifyType(const types::Flight& flight) {
    if (this->m_airport == flight.flightPlan().origin() && this->m_airport == flight.flightPlan().destination()) {
        if (true == flight.airborne())
            return types::Flight::Type::Arrival;
        else
            return types::Flight::Type::Departure;
    }
    else if (this->m_airport == flight.flightPlan().origin()) {
        return types::Flight::Type::Departure;
    }
    else if (this->m_airport == flight.flightPlan().destination()) {
        return types::Flight::Type::Arrival;
    }
    else {
        return types::Flight::Type::Unknown;
    }
}
