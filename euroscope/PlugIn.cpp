/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the EuroScope plug-in definition
 */

#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"

#include <filesystem>
#include <fstream>
#include <regex>

#include <Windows.h>
#include <shlwapi.h>

#define CURL_STATICLIB 1
#include <curl/curl.h>

#include <helper/String.h>
#include <management/PdcControl.h>
#include <surveillance/FlightPlanControl.h>
#include <system/ConfigurationRegistry.h>
#include <version.h>

#include "Converter.h"
#include "ui/MessageViewerWindow.h"
#include "ui/PdcDepartureClearanceWindow.h"
#include "ui/PdcMessageViewerWindow.h"
#include "PlugIn.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace fs = std::filesystem;

using namespace topskytower;
using namespace topskytower::euroscope;
using namespace topskytower::types;

static Gdiplus::GdiplusStartupInput __gdiStartupInput;
static ULONG_PTR                    __gdiplusToken;

PlugIn::PlugIn() :
        EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
                                 PLUGIN_NAME,
                                 PLUGIN_VERSION,
                                 PLUGIN_DEVELOPER,
                                 PLUGIN_COPYRIGHT),
        m_errorMode(false),
        m_settingsPath(),
        m_screens(),
        m_uiCallback(),
        m_pdcNotificationSound() {
    this->DisplayUserMessage("Message", PLUGIN_NAME, (std::string(PLUGIN_NAME) + " " + PLUGIN_VERSION + " loaded").c_str(),
                             false, false, false, false, false);

    if (0 != curl_global_init(CURL_GLOBAL_ALL)) {
        this->DisplayUserMessage("Message", PLUGIN_NAME, "Unable to initialize the network stack!",
                                 true, true, true, true, false);
        this->m_errorMode = true;
        return;
    }

    /* unable to initialize GDI+ */
    if (Gdiplus::Ok != Gdiplus::GdiplusStartup(&__gdiplusToken, &__gdiStartupInput, nullptr)) {
        this->DisplayUserMessage("Message", PLUGIN_NAME, "Unable to initialize the rendering system!",
                                 true, true, true, true, false);
        this->m_errorMode = true;
        return;
    }

    char path[MAX_PATH] = { 0 };
    GetModuleFileNameA((HINSTANCE)&__ImageBase, path, _countof(path));
    PathRemoveFileSpecA(path);
    this->m_settingsPath = path;

    system::ConfigurationRegistry::instance().configure(this->m_settingsPath, system::ConfigurationRegistry::UpdateType::All);

    this->RegisterTagItemType("Handoff frequency", static_cast<int>(PlugIn::TagItemElement::HandoffFrequency));
    this->RegisterTagItemType("Manually alerts 0", static_cast<int>(PlugIn::TagItemElement::ManuallyAlerts0));
    this->RegisterTagItemType("Manually alerts 1", static_cast<int>(PlugIn::TagItemElement::ManuallyAlerts1));
    this->RegisterTagItemType("Manually alerts 2", static_cast<int>(PlugIn::TagItemElement::ManuallyAlerts2));
    this->RegisterTagItemType("Flight marker", static_cast<int>(PlugIn::TagItemElement::FlightMarker));
    this->RegisterTagItemType("PDC indicator", static_cast<int>(PlugIn::TagItemElement::PdcIndicator));
    this->RegisterTagItemType("SID step climb indicator", static_cast<int>(PlugIn::TagItemElement::SIDStepClimbIndicator));
    this->RegisterTagItemType("Flight plan check", static_cast<int>(PlugIn::TagItemElement::FlighPlanCheck));
    this->RegisterTagItemType("Assigned stand", static_cast<int>(PlugIn::TagItemElement::AircraftStand));
    this->RegisterTagItemType("Departure Ground status", static_cast<int>(PlugIn::TagItemElement::DepartureGroundStatus));
    this->RegisterTagItemType("Arrival Ground status", static_cast<int>(PlugIn::TagItemElement::ArrivalGroundStatus));
    this->RegisterTagItemType("Surveillance alerts", static_cast<int>(PlugIn::TagItemElement::SurveillanceAlerts));

    this->RegisterTagItemFunction("Menu bar", static_cast<int>(PlugIn::TagItemFunction::AircraftControlMenuBar));
    this->RegisterTagItemFunction("PDC menu bar", static_cast<int>(PlugIn::TagItemFunction::PdcMenu));
    this->RegisterTagItemFunction("FP check menu", static_cast<int>(PlugIn::TagItemFunction::FlightPlanCheckMenu));
    this->RegisterTagItemFunction("Stand menu", static_cast<int>(PlugIn::TagItemFunction::StandControlMenu));
    this->RegisterTagItemFunction("Departure Ground status menu", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusMenu));
    this->RegisterTagItemFunction("Arrival Ground status menu", static_cast<int>(PlugIn::TagItemFunction::ArrivalGroundStatusMenu));
    this->RegisterTagItemFunction("Surveillance visualization trigger", static_cast<int>(PlugIn::TagItemFunction::SurveillanceAlertVisualization));
    this->RegisterTagItemFunction("Draw departure route", static_cast<int>(PlugIn::TagItemFunction::DepartureRouteDraw));

    /* search for the sound file and register the PDC sound callback */
    for (auto& entry : fs::recursive_directory_iterator(path)) {
        if (true == fs::is_regular_file(entry) && "TopSkyTower-Pdc.wav" == entry.path().filename().string()) {
            this->m_pdcNotificationSound = entry.path().string();
            management::PdcControl::instance().registerNotificationCallback(this, &PlugIn::pdcMessageReceived);
            break;
        }
    }
}

PlugIn::~PlugIn() {
    this->m_screens.clear();

    if (false == this->m_errorMode) {
        Gdiplus::GdiplusShutdown(__gdiplusToken);
        curl_global_cleanup();
    }
}

const std::string& PlugIn::settingsPath() const {
    return this->m_settingsPath;
}

static __inline void __markRunwayActive(EuroScopePlugIn::CSectorElement& runway, const std::string& airport,
                                        std::map<std::string, std::list<std::string>>& departureRunways,
                                        std::map<std::string, std::list<std::string>>& arrivalRunways, int idx) {
    if (true == runway.IsElementActive(true, idx))
        departureRunways[airport].push_back(runway.GetRunwayName(idx));

    if (true == runway.IsElementActive(false, idx))
        arrivalRunways[airport].push_back(runway.GetRunwayName(idx));
}

void PlugIn::OnAirportRunwayActivityChanged() {
    auto configuration = system::ConfigurationRegistry::instance().runtimeConfiguration();

    configuration.activeDepartureRunways.clear();
    configuration.activeArrivalRunways.clear();

    EuroScopePlugIn::CSectorElement rwy;
    for (rwy = this->SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_RUNWAY); true == rwy.IsValid();
        rwy = this->SectorFileElementSelectNext(rwy, EuroScopePlugIn::SECTOR_ELEMENT_RUNWAY)) {
        /* remove leading and trailing whitespaces */
        std::string airport(rwy.GetAirportName());
        airport = std::regex_replace(airport, std::regex("^ +| +$|( ) +"), "$1");

        __markRunwayActive(rwy, airport, configuration.activeDepartureRunways, configuration.activeArrivalRunways, 0);
        __markRunwayActive(rwy, airport, configuration.activeDepartureRunways, configuration.activeArrivalRunways, 1);
    }

    system::ConfigurationRegistry::instance().setRuntimeConfiguration(configuration);
}

EuroScopePlugIn::CRadarScreen* PlugIn::OnRadarScreenCreated(const char* displayName, bool needsRadarContent, bool geoReferenced,
                                                            bool canBeSaved, bool canBeCreated) {
    (void)needsRadarContent;
    (void)geoReferenced;
    (void)canBeSaved;
    (void)canBeCreated;
    (void)displayName;

    if (false == this->m_errorMode) {
        if (0 == this->m_screens.size())
            this->OnAirportRunwayActivityChanged();

        this->m_screens.push_back(new RadarScreen());
        return this->m_screens.back();
    }

    return nullptr;
}

bool PlugIn::visualizeManuallyAlerts(const types::Flight& flight, int idx, char itemString[16]) {
    int localOffset = 0, localIdx = 0;
    bool inserted = false;

    if (true == flight.onMissedApproach()) {
        localOffset += 9;
        if (localIdx == idx) {
            std::strcat(itemString, "M/A ");
            inserted = true;
        }
    }

    if (true == flight.irregularHandoff()) {
        localOffset += 7;

        if (16 <= localOffset) {
            localIdx += localOffset / 16;
            localOffset = 0;
            if (idx < localIdx)
                return inserted;
        }

        if (localIdx == idx) {
            std::strcat(itemString, "IRREG ");
            inserted = true;
        }
    }

    if (true == flight.establishedOnILS()) {
        localOffset += 5;

        if (16 <= localOffset) {
            localIdx += localOffset / 16;
            localOffset = 0;
            if (idx < localIdx)
                return inserted;
        }

        if (localIdx == idx) {
            std::strcat(itemString, "EST ");
            inserted = true;
        }
    }

    return inserted;
}

bool PlugIn::summarizeFlightPlanCheck(const std::list<surveillance::FlightPlanControl::ErrorCode>& codes,
                                      char* itemString, int* colorCode) {
    /* something went wrong */
    if (0 == codes.size()) {
        std::strcpy(itemString, "UNK");
        *colorCode = EuroScopePlugIn::TAG_COLOR_EMERGENCY;
    }
    /* more than one error found */
    else if (1 != codes.size()) {
        std::strcpy(itemString, "ERR");
        *colorCode = EuroScopePlugIn::TAG_COLOR_EMERGENCY;
    }
    /* search the correct abbreviation of the error */
    else {
        std::string code;

        /* get the correct error code */
        switch (codes.front()) {
        case surveillance::FlightPlanControl::ErrorCode::VFR:
            code = "VFR";
            break;
        case surveillance::FlightPlanControl::ErrorCode::NoError:
            code = "OK";
            break;
        case surveillance::FlightPlanControl::ErrorCode::Route:
            code = "RTE";
            break;
        case surveillance::FlightPlanControl::ErrorCode::DepartureRoute:
            code = "SID";
            break;
        case surveillance::FlightPlanControl::ErrorCode::EngineType:
            code = "ENG";
            break;
        case surveillance::FlightPlanControl::ErrorCode::Navigation:
            code = "NAV";
            break;
        case surveillance::FlightPlanControl::ErrorCode::Transponder:
            code = "XPD";
            break;
        case surveillance::FlightPlanControl::ErrorCode::FlightLevel:
            code = "FL";
            break;
        case surveillance::FlightPlanControl::ErrorCode::EvenOddLevel:
            code = "E/O";
            break;
        default:
            code = "UNK";
            break;
        }

        std::strcpy(itemString, code.c_str());
        if ("OK" == code || "VFR" == code) {
            *colorCode = EuroScopePlugIn::TAG_COLOR_DEFAULT;
            return true;
        }
        else {
            *colorCode = EuroScopePlugIn::TAG_COLOR_EMERGENCY;
        }
    }

    return false;
}

RadarScreen* PlugIn::findScreenAndFlight(const std::string& callsign, types::Flight& flight) const {
    std::list<RadarScreen*> candidates;

    /* test which screen is relevant */
    for (const auto& screen : std::as_const(this->m_screens)) {
        if (true == screen->flightRegistry().flightExists(callsign)) {
            /* copy it once -> the data is consistent over all screens */
            if (flight.callsign() != callsign)
                flight = screen->flightRegistry().flight(callsign);

            if (true == screen->isInitialized() && true == screen->sectorControl().isInSector(flight))
                candidates.push_back(screen);
        }
    }

    /* use the newest screen, if we have more than one candidate */
    if (1 <= candidates.size()) {
        candidates.sort([](RadarScreen* screen0, RadarScreen* screen1) {
            return screen0->lastRenderingTime() > screen1->lastRenderingTime();
        });

        flight = candidates.front()->flightRegistry().flight(callsign);
        RadarScreen* retval = candidates.front();
        return retval;
    }
    else {
        return nullptr;
    }
}

void PlugIn::OnGetTagItem(EuroScopePlugIn::CFlightPlan flightPlan, EuroScopePlugIn::CRadarTarget radarTarget,
                          int itemCode, int tagData, char itemString[16], int* colorCode, COLORREF* rgb,
                          double* fontSize) {
    (void)flightPlan;
    (void)tagData;
    (void)rgb;
    (void)fontSize;

    /* do not handle invalid radar targets */
    if (true == this->m_errorMode || false == radarTarget.IsValid())
        return;

    /* initialize default values */
    std::string callsign(radarTarget.GetCallsign());
    itemString[0] = '\0';
    *colorCode = EuroScopePlugIn::TAG_COLOR_DEFAULT;

    /* get the screen and the converted flight information */
    types::Flight flight;
    auto flightScreen = this->findScreenAndFlight(callsign, flight);
    if (nullptr == flightScreen)
        return;

    switch (static_cast<PlugIn::TagItemElement>(itemCode)) {
    case PlugIn::TagItemElement::HandoffFrequency:
        if (true == flightScreen->sectorControl().handoffRequired(flight)) {
            auto& info = flightScreen->sectorControl().handoffSector(flight);
            std::string msg = info.identifier() + " " + info.primaryFrequency();

            std::strncpy(itemString, msg.c_str(), 16 < (msg.length() + 1) ? 16 : msg.length() + 1);
            *colorCode = EuroScopePlugIn::TAG_COLOR_NOTIFIED;
        }
        break;
    case PlugIn::TagItemElement::ManuallyAlerts0:
        if (true == PlugIn::visualizeManuallyAlerts(flight, 0, itemString))
            *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
        break;
    case PlugIn::TagItemElement::ManuallyAlerts1:
        if (true == PlugIn::visualizeManuallyAlerts(flight, 1, itemString))
            *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
        break;
    case PlugIn::TagItemElement::ManuallyAlerts2:
        if (true == PlugIn::visualizeManuallyAlerts(flight, 2, itemString))
            *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
        break;
    case PlugIn::TagItemElement::FlightMarker:
        if (true == flight.markedByController()) {
            std::string msg("\u2022 ");
            std::memcpy(itemString, msg.c_str(), msg.length() + 1);
            *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
        }
        break;
    case PlugIn::TagItemElement::PdcIndicator:
    {
        std::string msg("\u2022 ");
        *fontSize = 4.0f;
        std::memcpy(itemString, msg.c_str(), msg.length() + 1);

        if (true == management::PdcControl::instance().messagesAvailable(flight))
            *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
        else
            *colorCode = EuroScopePlugIn::TAG_COLOR_NON_CONCERNED;

        break;
    }
    case PlugIn::TagItemElement::SIDStepClimbIndicator:
        if (flight.flightPlan().type() == types::FlightPlan::Type::IFR) {
            const auto& config = system::ConfigurationRegistry::instance().airportConfiguration(flight.flightPlan().origin());
            auto sidIt = config.sids.find(flight.flightPlan().departureRoute());

            if (config.sids.cend() != sidIt) {
                if (true == sidIt->second.containsStepClimbs) {
                    std::strcpy(itemString, "YES");
                    *colorCode = EuroScopePlugIn::TAG_COLOR_DEFAULT;
                }
                else {
                    std::strcpy(itemString, "NO");
                    *colorCode = EuroScopePlugIn::TAG_COLOR_NON_CONCERNED;
                }
            }
            else {
                std::strcpy(itemString, "UNK");
                *colorCode = EuroScopePlugIn::TAG_COLOR_EMERGENCY;
            }
        }
        else {
            std::strcpy(itemString, "VFR");
            *colorCode = EuroScopePlugIn::TAG_COLOR_NON_CONCERNED;
        }
        break;
    case PlugIn::TagItemElement::FlighPlanCheck:
    {
        bool finalizeRoute = false;

        surveillance::FlightPlanControl::instance().validate(flight);
        if (true == surveillance::FlightPlanControl::instance().overwritten(flight.callsign())) {
            std::strcpy(itemString, "OK");
            *colorCode = EuroScopePlugIn::TAG_COLOR_DEFAULT;
            finalizeRoute = true;
        }
        else {
            finalizeRoute = PlugIn::summarizeFlightPlanCheck(surveillance::FlightPlanControl::instance().errorCodes(flight.callsign()),
                                                             itemString, colorCode);
        }

        /* check if the flight is controlled by me */
        if (false == flightScreen->sectorControl().isInOwnSector(flight) && false == flight.isTracked())
            finalizeRoute = false;

        if (true == finalizeRoute && types::FlightPlan::Type::VFR != flight.flightPlan().type()) {
            std::string departure = flight.flightPlan().departureRoute() + "/";
            departure += radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().GetDepartureRwy();
            std::string route(radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().GetRoute());

            /* check if the route is already configured */
            if (std::string::npos == route.find(departure, 0)) {
                auto pos = flight.flightPlan().departureRoute().find_first_of("0123456789", 0);
                std::string firstWaypoint = flight.flightPlan().departureRoute().substr(0, pos);
                auto entries = helper::String::splitString(route, " ");

                /* find the first waypoint and ignore all before it */
                auto wpIt = std::find(entries.cbegin(), entries.cend(), firstWaypoint);

                /* create the new route */
                std::string newRoute(departure + " ");
                for (auto routeIt = wpIt; entries.cend() != routeIt; ++routeIt)
                    newRoute += *routeIt + " ";

                /* write into the flight plan */
                radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().SetRoute(newRoute.c_str());
                radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().AmendFlightPlan();
            }

            /* update the initial clearance limit */
            auto& config = system::ConfigurationRegistry::instance().airportConfiguration(flight.flightPlan().origin());
            auto sidIt = config.sids.find(flight.flightPlan().departureRoute());
            auto esPlan = radarTarget.GetCorrelatedFlightPlan();

            auto clearedFL = static_cast<int>(sidIt->second.clearanceLimit.convert(types::feet));
            if (esPlan.GetControllerAssignedData().GetClearedAltitude() != clearedFL)
                esPlan.GetControllerAssignedData().SetClearedAltitude(clearedFL);
        }

        break;
    }
    case PlugIn::TagItemElement::AircraftStand:
    {
        auto stand = flightScreen->standControl().stand(flight);
        if (0 != stand.length()) {
            /* validate that no other controller published a stand already */
            std::string strip = radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand));
            if (0 != strip.length()) {
                auto split = helper::String::splitString(strip, "/");
                /* validate that it is the stand-message */
                if (3 == split.size() && "s" == split[0] && "s" == split[2]) {
                    /* someone else published this stand -> take it */
                    if (stand != split[1]) {
                        flightScreen->standControl().assignManually(flight, split[1]);
                        stand = split[1];
                    }
                }
            }

            std::strcpy(itemString, stand.c_str());

            /* define the color */
            if (true == flightScreen->standControl().standIsBlocked(stand)) {
                *colorCode = EuroScopePlugIn::TAG_COLOR_EMERGENCY;
            }
            else if (types::Flight::Type::Arrival == flight.type()) {
                /* check if we have to publish the stand */
                auto esStrip = radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand));
                std::string expected = "s/" + stand + "/s";

                if (esStrip == expected)
                    *colorCode = EuroScopePlugIn::TAG_COLOR_DEFAULT;
                else
                    *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
            }
            else {
                *colorCode = EuroScopePlugIn::TAG_COLOR_DEFAULT;
            }
        }
        break;
    }
    case PlugIn::TagItemElement::DepartureGroundStatus:
        *colorCode = EuroScopePlugIn::TAG_COLOR_DEFAULT;
        switch (flight.flightPlan().departureFlag()) {
        case types::FlightPlan::AtcCommand::StartUp:
            std::strcpy(itemString, "ST-UP");
            break;
        case types::FlightPlan::AtcCommand::Deicing:
            std::strcpy(itemString, "DEICE");
            break;
        case types::FlightPlan::AtcCommand::Pushback:
            std::strcpy(itemString, "PUSH");
            break;
        case types::FlightPlan::AtcCommand::TaxiOut:
            std::strcpy(itemString, "TAXI");
            break;
        case types::FlightPlan::AtcCommand::LineUp:
            std::strcpy(itemString, "LI-UP");
            break;
        case types::FlightPlan::AtcCommand::Departure:
            std::strcpy(itemString, "DEPA");
            break;
        default:
            break;
        }
        break;
    case PlugIn::TagItemElement::ArrivalGroundStatus:
        *colorCode = EuroScopePlugIn::TAG_COLOR_DEFAULT;
        switch (flight.flightPlan().arrivalFlag()) {
        case types::FlightPlan::AtcCommand::Approach:
            std::strcpy(itemString, "APPR");
            break;
        case types::FlightPlan::AtcCommand::Land:
            std::strcpy(itemString, "LAND");
            break;
        case types::FlightPlan::AtcCommand::TaxiIn:
            std::strcpy(itemString, "TAXI");
            break;
        case types::FlightPlan::AtcCommand::GoAround:
            std::strcpy(itemString, "GO-AR");
            break;
        default:
            break;
        }
        break;
    case PlugIn::TagItemElement::SurveillanceAlerts:
        *colorCode = EuroScopePlugIn::TAG_COLOR_EMERGENCY;
        itemString[0] = '\0';

        if (true == flightScreen->stcdControl().ntzViolation(flight))
            std::strcat(itemString, "NTZ");
        else if (true == flightScreen->stcdControl().separationLoss(flight))
            std::strcat(itemString, "STC");
        else if (true == flightScreen->ariwsControl().runwayIncursionWarning(flight))
            std::strcat(itemString, "RIW");
        else if (true == flightScreen->cmacControl().conformanceMonitoringAlert(flight))
            std::strcat(itemString, "CMA");
        else if (true == flightScreen->mtcdControl().conflictsExist(flight))
            std::strcat(itemString, "MTC");

        break;
    default:
        break;
    }
}

void PlugIn::handleHandoffPerform(POINT point, RECT area, const types::Flight& flight, bool tracked, RadarScreen* screen) {
    auto radarTarget = this->RadarTargetSelectASEL();

    /* found the correct screen with the handoff */
    if (true == screen->sectorControl().handoffRequired(flight)) {
        auto controllers = screen->sectorControl().handoffStations(flight);

        /* check if handoff or a release is needed */
        if (1 == controllers.size()) {
            /* handoff to unicom */
            if (true == tracked) {
                if (0 == controllers.front().size())
                    radarTarget.GetCorrelatedFlightPlan().EndTracking();
                else
                    radarTarget.GetCorrelatedFlightPlan().InitiateHandoff(controllers.front().c_str());
            }
            screen->sectorControl().handoffPerformed(flight);
        }
        else {
            RadarScreen::EuroscopeEvent eventEntry = {
                static_cast<int>(PlugIn::TagItemFunction::HandoffControllerSelectEvent),
                flight.callsign(),
                "",
                point,
                area
            };
            screen->registerEuroscopeEvent(std::move(eventEntry));
        }
    }
}

void PlugIn::updateManuallyAlerts(EuroScopePlugIn::CRadarTarget& radarTarget, const std::string& marker) {
    std::string scratchPad(radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetScratchPadString());

    std::size_t pos = scratchPad.find(marker);
    if (std::string::npos != pos)
        scratchPad.erase(pos, marker.length());
    else
        scratchPad += marker;

    radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetScratchPadString(scratchPad.c_str());
}

void PlugIn::updateFlightStrip(EuroScopePlugIn::CRadarTarget& radarTarget, RadarScreen* screen, int idx, const std::string& message) {
    if (8 < idx)
        return;

    std::string entry(radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetFlightStripAnnotation(idx));

    std::size_t pos = entry.find(message);
    if (std::string::npos != pos) {
        entry.erase(pos, message.length());
        radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(idx, entry.c_str());
    }
    else {
        radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(idx, message.c_str());
    }

    /* publish the strip to all available controllers */
    for (const auto& sector : screen->sectorControl().findAllRelatedControllers()) {
        /* create the callsign */
        std::string callsign = sector.prefix() + "_";
        if (0 != sector.midfix().length())
            callsign += sector.midfix() + "_";
        callsign += sector.suffix();

        radarTarget.GetCorrelatedFlightPlan().PushFlightStrip(callsign.c_str());
    }

    for (auto& screenElem : this->m_screens) {
        auto updatedFlight = Converter::convert(radarTarget, *screenElem);
        screenElem->flightRegistry().updateFlight(updatedFlight);
    }
}

RadarScreen* PlugIn::findLastActiveScreen() {
    std::chrono::system_clock::time_point newest;
    RadarScreen* retval = nullptr;

    for (auto& screen : this->m_screens) {
        if (screen->lastRenderingTime() >= newest) {
            newest = screen->lastRenderingTime();
            retval = screen;
        }
    }

    return retval;
}

std::string PlugIn::flightPlanCheckResultLog(const std::list<surveillance::FlightPlanControl::ErrorCode>& codes) {
    std::string retval;

    if (0 == codes.size()) {
        return "Completely wrong or no flight plan received!";
    }
    else {
        for (const auto& code : std::as_const(codes)) {
            switch (code) {
            case surveillance::FlightPlanControl::ErrorCode::VFR:
                retval += "VFR flight!\n";
                break;
            case surveillance::FlightPlanControl::ErrorCode::NoError:
                retval += "Valid flight plan - No errors found!\n";
                break;
            case surveillance::FlightPlanControl::ErrorCode::Route:
                retval += "No or an invalid route received!\n";
                break;
            case surveillance::FlightPlanControl::ErrorCode::DepartureRoute:
                retval += "Unknown SID found!\n";
                break;
            case surveillance::FlightPlanControl::ErrorCode::EngineType:
                retval += "ACs engine type is invalid!\n";
                break;
            case surveillance::FlightPlanControl::ErrorCode::Navigation:
                retval += "Navigation capabilities (i.e. RNAV) insufficient!\n";
                break;
            case surveillance::FlightPlanControl::ErrorCode::Transponder:
                retval += "AC does not have a transponder!\n";
                break;
            case surveillance::FlightPlanControl::ErrorCode::FlightLevel:
                retval += "Requested flight level is not between min/max allowed FL!\n";
                break;
            case surveillance::FlightPlanControl::ErrorCode::EvenOddLevel:
                retval += "Requested flight level is wrong based on even-odd-rule!\n";
                break;
            default:
                retval += "Unknown error found!\n";
                break;
            }
        }
    }

    return retval;
}

void PlugIn::updateGroundStatus(EuroScopePlugIn::CRadarTarget target, const std::string_view& view,
                                RadarScreen* screen, const types::Flight& flight, bool arrival) {
    std::uint8_t mask;

    if (false == arrival) {
        mask = static_cast<std::uint8_t>(flight.flightPlan().arrivalFlag());

        if ("ST-UP" == view) {
            mask |= static_cast<std::uint8_t>(types::FlightPlan::AtcCommand::StartUp);
            target.GetCorrelatedFlightPlan().GetControllerAssignedData().SetScratchPadString("ST-UP");
        }
        else if ("PUSH" == view) {
            mask |= static_cast<std::uint8_t>(types::FlightPlan::AtcCommand::Pushback);
            target.GetCorrelatedFlightPlan().GetControllerAssignedData().SetScratchPadString("PUSH");
        }
        else if ("TAXI" == view) {
            mask |= static_cast<std::uint8_t>(types::FlightPlan::AtcCommand::TaxiOut);
            target.GetCorrelatedFlightPlan().GetControllerAssignedData().SetScratchPadString("TAXI");
        }
        else if ("DEICE" == view) {
            mask |= static_cast<std::uint8_t>(types::FlightPlan::AtcCommand::Deicing);
        }
        else if ("LI-UP" == view) {
            mask |= static_cast<std::uint8_t>(types::FlightPlan::AtcCommand::LineUp);
        }
        else if ("DEPA" == view) {
            mask |= static_cast<std::uint8_t>(types::FlightPlan::AtcCommand::Departure);
            target.GetCorrelatedFlightPlan().GetControllerAssignedData().SetScratchPadString("DEPA");
        }
    }
    else {
        mask = static_cast<std::uint8_t>(flight.flightPlan().departureFlag());

        if ("APPR" == view) {
            mask |= static_cast<std::uint8_t>(types::FlightPlan::AtcCommand::Approach);
            if (true == flight.onMissedApproach())
                PlugIn::updateManuallyAlerts(target, "MISAP_");
        }
        else if ("LAND" == view) {
            mask |= static_cast<std::uint8_t>(types::FlightPlan::AtcCommand::Land);
            if (true == flight.onMissedApproach())
                PlugIn::updateManuallyAlerts(target, "MISAP_");
        }
        else if ("TAXI" == view) {
            mask |= static_cast<std::uint8_t>(types::FlightPlan::AtcCommand::TaxiIn);
            if (true == flight.onMissedApproach())
                PlugIn::updateManuallyAlerts(target, "MISAP_");
        }
        else if ("GO-AR" == view) {
            mask |= static_cast<std::uint8_t>(types::FlightPlan::AtcCommand::GoAround);
            if (false == flight.onMissedApproach())
                PlugIn::updateManuallyAlerts(target, "MISAP_");
        }
    }

    std::string annotation = "c/" + std::to_string(mask) + "/c";
    this->updateFlightStrip(target, screen, static_cast<int>(PlugIn::AnnotationIndex::AtcCommand), annotation);
}

void PlugIn::OnFunctionCall(int functionId, const char* itemString, POINT pt, RECT area) {
    if (true == this->m_errorMode)
        return;

    if (PlugIn::TagItemFunction::UiElementIds < static_cast<PlugIn::TagItemFunction>(functionId)) {
        switch (static_cast<PlugIn::TagItemFunction>(functionId)) {
        case PlugIn::TagItemFunction::UiEditTextRequest:
            if (nullptr != this->m_uiCallback)
                this->OpenPopupEdit(area, static_cast<int>(PlugIn::TagItemFunction::UiEditTextResponse), itemString);
            break;
        case PlugIn::TagItemFunction::UiEditTextResponse:
            if (nullptr != this->m_uiCallback) {
                this->m_uiCallback(itemString);
                this->m_uiCallback = nullptr;
            }
            break;
        default:
            break;
        }

        return;
    }

    auto radarTarget = this->RadarTargetSelectASEL();
    if (false == radarTarget.IsValid())
        return;
    std::string callsign(radarTarget.GetCallsign());

    /* check if we are tracking and search the flight */
    types::Flight flight;
    auto flightScreen = this->findScreenAndFlight(callsign, flight);

    /* nothing to do */
    if (nullptr == flightScreen)
        return;

    switch (static_cast<PlugIn::TagItemFunction>(functionId)) {
    case PlugIn::TagItemFunction::AircraftControlMenuBar:
        this->OpenPopupList(area, "Aircraft menu", 2);

        /* define the menu bar for the tracking functions */
        if (true == system::ConfigurationRegistry::instance().systemConfiguration().trackingOnGround &&
            true == radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe())
        {
            this->AddPopupListElement("Transfer", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                      false == flightScreen->sectorControl().handoffRequired(flight), false);
            this->AddPopupListElement("Man. Transfer", "", static_cast<int>(PlugIn::TagItemFunction::HandoffSectorChangeEvent),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                      false == flightScreen->sectorControl().handoffPossible(flight), false);
            this->AddPopupListElement("Release", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform));
            this->AddPopupListElement("CTR change", "", static_cast<int>(PlugIn::TagItemFunction::SectorControllerHandover),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                      false == flightScreen->sectorControl().sectorHandoverPossible() || false == flight.isTracked(), false);
        }
        else {
            std::string_view ctrCallsign = radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerCallsign();
            std::string_view trackedCallsign = radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerCallsign();

            if (true == system::ConfigurationRegistry::instance().systemConfiguration().trackingOnGround) {
                this->AddPopupListElement("Accept", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform),
                                          false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                          this->ControllerMyself().GetCallsign() != ctrCallsign, false);
                this->AddPopupListElement("Assume", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform),
                                          false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, 0 != trackedCallsign.length(), false);
                this->AddPopupListElement("Refuse", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform),
                                          false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                          this->ControllerMyself().GetCallsign() != ctrCallsign, false);
            }
            this->AddPopupListElement("Handoff", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                      false == flightScreen->sectorControl().handoffRequired(flight), false);
        }
        this->AddPopupListElement("-------------", "-", 0, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true, false);

        /* define the menu bar for the manually alerts */
        if (true == radarTarget.GetCorrelatedFlightPlan().IsValid()) {
            this->AddPopupListElement("M/A", (true == flight.onMissedApproach() ? "X" : ""),
                                      static_cast<int>(PlugIn::TagItemFunction::AircraftControlSignal), false,
                                      EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true == flight.isTrackedByOther(), false);
            this->AddPopupListElement("Irreg", (true == flight.irregularHandoff() ? "X" : ""),
                                      static_cast<int>(PlugIn::TagItemFunction::AircraftControlSignal), false,
                                      EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true == flight.isTrackedByOther(), false);
            this->AddPopupListElement("Est", (true == flight.establishedOnILS() ? "X" : ""),
                                      static_cast<int>(PlugIn::TagItemFunction::AircraftControlSignal), false,
                                      EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true == flight.isTrackedByOther(), false);
            this->AddPopupListElement("Mark", (true == flight.markedByController() ? "X" : ""),
                                      static_cast<int>(PlugIn::TagItemFunction::AircraftControlSignal), false,
                                      EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true == flight.isTrackedByOther(), false);
        }

        break;
    case PlugIn::TagItemFunction::AircraftControlSignal:
        if (false == flight.isTrackedByOther()) {
            if (0 == std::strncmp(itemString, "M/A", 7))
                PlugIn::updateManuallyAlerts(radarTarget, "MISAP_");
            else if (0 == std::strncmp(itemString, "Irreg", 5))
                PlugIn::updateManuallyAlerts(radarTarget, "IRREG_");
            else if (0 == std::strncmp(itemString, "Est", 3))
                PlugIn::updateManuallyAlerts(radarTarget, "EST_");
            else if (0 == std::strncmp(itemString, "Mark", 4))
                this->updateFlightStrip(radarTarget, flightScreen, 7, "K");
        }
        break;
    case PlugIn::TagItemFunction::HandoffPerform:
        if (0 == std::strncmp(itemString, "Release", 7)) {
            radarTarget.GetCorrelatedFlightPlan().EndTracking();

            /* check if a handoff to UNICOM is ongoing */
            if (true == flightScreen->sectorControl().handoffRequired(flight)) {
                auto controllers = flightScreen->sectorControl().handoffStations(flight);
                if (0 == controllers.front().size())
                    flightScreen->sectorControl().handoffPerformed(flight);
            }
        }
        else if (0 == std::strncmp(itemString, "Assume", 6)) {
            radarTarget.GetCorrelatedFlightPlan().StartTracking();
        }
        else if (0 == std::strncmp(itemString, "Accept", 6)) {
            radarTarget.GetCorrelatedFlightPlan().AcceptHandoff();
        }
        else if (0 == std::strncmp(itemString, "Refuse", 6)) {
            radarTarget.GetCorrelatedFlightPlan().RefuseHandoff();
        }
        else {
            this->handleHandoffPerform(pt, area, flight, flight.isTracked(), flightScreen);
        }
        break;
    case PlugIn::TagItemFunction::HandoffControllerSelectEvent:
        if (true == flightScreen->sectorControl().handoffRequired(flight)) {
            auto controllers = flightScreen->sectorControl().handoffStations(flight);
            auto& info = flightScreen->sectorControl().handoffSector(flight);
            this->OpenPopupList(area, "Handoff To", 2);

            for (const auto& controller : std::as_const(controllers)) {
                this->AddPopupListElement(controller.c_str(), info.primaryFrequency().c_str(),
                                          static_cast<int>(PlugIn::TagItemFunction::HandoffControllerSelect));
            }
        }
        break;
    case PlugIn::TagItemFunction::HandoffControllerSelect:
        if (true == flightScreen->sectorControl().handoffRequired(flight)) {
            if (true == flight.isTracked())
                radarTarget.GetCorrelatedFlightPlan().InitiateHandoff(itemString);
            flightScreen->sectorControl().handoffPerformed(flight);
        }
        break;
    case PlugIn::TagItemFunction::HandoffSectorChangeEvent:
        if (true == flightScreen->sectorControl().handoffRequired(flight) || true == flightScreen->sectorControl().handoffPossible(flight)) {
            RadarScreen::EuroscopeEvent eventEntry = {
                static_cast<int>(PlugIn::TagItemFunction::HandoffSectorChange),
                callsign,
                "",
                pt,
                area
            };
            flightScreen->registerEuroscopeEvent(std::move(eventEntry));
        }
        break;
    case PlugIn::TagItemFunction::HandoffSectorChange:
        if (true == flightScreen->sectorControl().handoffRequired(flight) || true == flightScreen->sectorControl().handoffPossible(flight)) {
            auto sectors = flightScreen->sectorControl().handoffSectors();

            this->OpenPopupList(area, "Handoff sectors", 2);
            for (const auto& sector : std::as_const(sectors)) {
                this->AddPopupListElement(sector.identifier().c_str(), sector.primaryFrequency().c_str(),
                                          static_cast<int>(PlugIn::TagItemFunction::HandoffSectorSelect));
            }
        }
        break;
    case PlugIn::TagItemFunction::HandoffSectorSelect:
        if (true == flightScreen->sectorControl().handoffRequired(flight) || true == flightScreen->sectorControl().handoffPossible(flight)) {
            flightScreen->sectorControl().handoffSectorSelect(flight, itemString);
            this->handleHandoffPerform(pt, area, flight, flight.isTracked(), flightScreen);
        }
        break;
    case PlugIn::TagItemFunction::SectorControllerHandover:
        if (true == flightScreen->sectorControl().sectorHandoverPossible()) {
            auto candidates = flightScreen->sectorControl().sectorHandoverCandidates();

            /* we have only one candidate */
            if (1 == candidates.size()) {
                std::string ctrCallsign = candidates.front().prefix() + "_";
                if (0 != candidates.front().midfix().length())
                    ctrCallsign += candidates.front().midfix() + "_";
                ctrCallsign += candidates.front().suffix();

                radarTarget.GetCorrelatedFlightPlan().InitiateHandoff(ctrCallsign.c_str());
            }
            else if (0 != candidates.size()) {
                RadarScreen::EuroscopeEvent esEvent = {
                    static_cast<int>(PlugIn::TagItemFunction::SectorControllerHandoverSelectEvent),
                    flight.callsign(),
                    "",
                    pt,
                    area
                };
                this->findLastActiveScreen()->registerEuroscopeEvent(std::move(esEvent));
            }
        }
        break;
    case PlugIn::TagItemFunction::SectorControllerHandoverSelectEvent:
        if (true == flightScreen->sectorControl().sectorHandoverPossible()) {
            auto candidates = flightScreen->sectorControl().sectorHandoverCandidates();

            this->OpenPopupList(area, "CTR Select", 1);
            for (const auto& candidate : std::as_const(candidates)) {
                std::string ctrCallsign = candidate.prefix() + "_";
                if (0 != candidate.midfix().length())
                    ctrCallsign += candidate.midfix() + "_";
                ctrCallsign += candidate.suffix();

                this->AddPopupListElement(ctrCallsign.c_str(), "",
                                          static_cast<int>(PlugIn::TagItemFunction::SectorControllerHandoverSelect));
            }
        }
        break;
    case PlugIn::TagItemFunction::SectorControllerHandoverSelect:
        radarTarget.GetCorrelatedFlightPlan().InitiateHandoff(itemString);
        break;
    case PlugIn::TagItemFunction::PdcMenu:
        if (false == flight.isTrackedByOther() && true == management::PdcControl::instance().airportLoggedIn(flightScreen->airportIcao())) {
            this->OpenPopupList(area, "PDC", 1);
            this->AddPopupListElement("Read", "", static_cast<int>(PlugIn::TagItemFunction::PdcReadMessage), false,
                                      EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                      false == management::PdcControl::instance().messagesAvailable(flight), false);
            this->AddPopupListElement("Send stand-by", "", static_cast<int>(PlugIn::TagItemFunction::PdcSendStandby));
            this->AddPopupListElement("Send clearance", "", static_cast<int>(PlugIn::TagItemFunction::PdcSendClearance), false,
                                      EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true == flight.flightPlan().clearanceFlag(), false);
        }
        break;
    case PlugIn::TagItemFunction::PdcReadMessage:
    {
        auto message = management::PdcControl::instance().nextMessage(flight);
        if (nullptr != message) {
            auto screen = this->findLastActiveScreen();

            auto viewer = new PdcMessageViewerWindow(screen, message);
            viewer->setActive(true);
        }
        break;
    }
    case PlugIn::TagItemFunction::PdcSendStandby:
        management::PdcControl::instance().sendStandbyMessage(flight);
        break;
    case PlugIn::TagItemFunction::PdcSendClearance:
    {
        auto screen = this->findLastActiveScreen();

        management::PdcControl::ClearanceMessagePtr message(new management::PdcControl::ClearanceMessage());
        message->sender = flight.flightPlan().origin();
        message->receiver = flight.callsign();
        message->destination = flight.flightPlan().destination();
        message->sid = flight.flightPlan().departureRoute();
        message->runway = radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().GetDepartureRwy();
        message->frequency = screen->sectorControl().ownSector().primaryFrequency();
        message->squawk = std::to_string(flight.flightPlan().assignedSquawk());

        if (this->GetTransitionAltitude() <= flight.flightPlan().clearanceLimit().convert(types::feet)) {
            auto limit = static_cast<int>(flight.flightPlan().clearanceLimit().convert(types::feet));
            limit /= 100;
            message->clearanceLimit = "FL" + std::to_string(limit);
        }
        else {
            message->clearanceLimit = std::to_string(static_cast<int>(flight.flightPlan().clearanceLimit().convert(types::feet)));
        }

        auto viewer = new PdcDepartureClearanceWindow(screen, message);
        viewer->setActive(true);

        break;
    }
    case PlugIn::TagItemFunction::FlightPlanCheckMenu:
    {
        /* check if the flight plan is valid */
        const auto& codes = surveillance::FlightPlanControl::instance().errorCodes(flight.callsign());
        bool isValid = false;
        if (1 == codes.size()) {
            if (surveillance::FlightPlanControl::ErrorCode::NoError == codes.front() ||
                surveillance::FlightPlanControl::ErrorCode::VFR == codes.front())
            {
                isValid = true;
            }
        }

        if (false == flight.isTrackedByOther()) {
            this->OpenPopupList(area, "FP check", 1);
            this->AddPopupListElement("Read results", "", static_cast<int>(PlugIn::TagItemFunction::FlightPlanCheckErrorLog));
            this->AddPopupListElement("Overwrite", "", static_cast<int>(PlugIn::TagItemFunction::FlightPlanCheckOverwrite), false,
                                      EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true == isValid, false);
        }
        break;
    }
    case PlugIn::TagItemFunction::FlightPlanCheckErrorLog:
    {
        auto messageLog = PlugIn::flightPlanCheckResultLog(surveillance::FlightPlanControl::instance().errorCodes(flight.callsign()));
        auto viewer = new MessageViewerWindow(this->findLastActiveScreen(), "FP check: " + flight.callsign(),
                                              messageLog);
        viewer->setActive(true);
        break;
    }
    case PlugIn::TagItemFunction::FlightPlanCheckOverwrite:
        surveillance::FlightPlanControl::instance().overwrite(flight.callsign());
        break;
    case PlugIn::TagItemFunction::StandControlMenu:
    {
        if (false == flight.isTrackedByOther()) {
            auto strip = radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand));
            auto stand = "s/" + flightScreen->standControl().stand(flight) + "/s";

            this->OpenPopupList(area, "Stand", 1);
            this->AddPopupListElement("Publish", "", static_cast<int>(PlugIn::TagItemFunction::StandControlPublish),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, stand == strip, false);
            this->AddPopupListElement("Automatic", "", static_cast<int>(PlugIn::TagItemFunction::StandControlAutomatic));
            this->AddPopupListElement("Manual", "", static_cast<int>(PlugIn::TagItemFunction::StandControlManualEvent));
        }
        break;
    }
    case PlugIn::TagItemFunction::StandControlPublish:
    {
        auto strip = radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand));
        auto stand = flightScreen->standControl().stand(flight);
        if (0 != stand.length()) {
            auto newStrip = "s/" + stand + "/s";
            /* do the check to increase the performance by avoiding useless network traffic */
            if (newStrip != strip)
                radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand), newStrip.c_str());
        }
        else if (0 != std::strlen(strip)) {
            /* reset the current annotation */
            radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand), "");
        }
        break;
    }
    case PlugIn::TagItemFunction::StandControlAutomatic:
        flightScreen->standControl().removeFlight(flight.callsign());
        if (0 != std::strlen(radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand))))
            radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand), "");
        break;
    case PlugIn::TagItemFunction::StandControlManualEvent:
    {
        RadarScreen::EuroscopeEvent esEvent = {
            static_cast<int>(PlugIn::TagItemFunction::StandControlManual),
            flight.callsign(),
            "",
            pt,
            area
        };
        flightScreen->registerEuroscopeEvent(std::move(esEvent));
        break;
    }
    case PlugIn::TagItemFunction::StandControlManual:
    {
        auto stands = flightScreen->standControl().allStands();
        this->OpenPopupList(area, "Stands", 1);
        for (const auto& stand : std::as_const(stands)) {
            this->AddPopupListElement(stand.first.c_str(), "", static_cast<int>(PlugIn::TagItemFunction::StandControlManualSelect),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true == stand.second, false);
        }
        break;
    }
    case PlugIn::TagItemFunction::StandControlManualSelect:
        flightScreen->standControl().assignManually(flight, itemString);
        if (0 != std::strlen(radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand))))
            radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand), "");
        break;
    case PlugIn::TagItemFunction::DepartureGroundStatusMenu:
        if (false == flight.isTrackedByOther()) {
            this->OpenPopupList(area, "Status", 1);
            this->AddPopupListElement("ST-UP", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
            this->AddPopupListElement("DEICE", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
            this->AddPopupListElement("PUSH", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
            this->AddPopupListElement("TAXI", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
            this->AddPopupListElement("LI-UP", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
            this->AddPopupListElement("DEPA", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
        }
        break;
    case PlugIn::TagItemFunction::DepartureGroundStatusSelect:
        this->updateGroundStatus(radarTarget, itemString, flightScreen, flight, false);
        flightScreen->flightRegistry().updateFlight(Converter::convert(radarTarget, *flightScreen));
        break;
    case PlugIn::TagItemFunction::ArrivalGroundStatusMenu:
        if (false == flight.isTrackedByOther()) {
            this->OpenPopupList(area, "Status", 1);
            this->AddPopupListElement("APPR", "", static_cast<int>(PlugIn::TagItemFunction::ArrivalGroundStatusSelect));
            this->AddPopupListElement("LAND", "", static_cast<int>(PlugIn::TagItemFunction::ArrivalGroundStatusSelect));
            this->AddPopupListElement("TAXI", "", static_cast<int>(PlugIn::TagItemFunction::ArrivalGroundStatusSelect));
            this->AddPopupListElement("GO-AR", "", static_cast<int>(PlugIn::TagItemFunction::ArrivalGroundStatusSelect));
        }
        break;
    case PlugIn::TagItemFunction::ArrivalGroundStatusSelect:
        this->updateGroundStatus(radarTarget, itemString, flightScreen, flight, true);
        flightScreen->flightRegistry().updateFlight(Converter::convert(radarTarget, *flightScreen));
        break;
    case PlugIn::TagItemFunction::SurveillanceAlertVisualization:
        flightScreen->activateSurveillanceVisualization(flight.callsign());
        break;
    case PlugIn::TagItemFunction::DepartureRouteDraw:
        flightScreen->activateDepartureRouteVisualization(flight.callsign());
        break;
    default:
        break;
    }
}

void PlugIn::OnNewMetarReceived(const char* station, const char* fullMetar) {
    auto configuration = system::ConfigurationRegistry::instance().runtimeConfiguration();
    auto split = helper::String::splitString(fullMetar, " ");

    /* find the wind entry */
    for (const auto& entry : std::as_const(split)) {
        /* find the wind entry */
        auto pos = entry.find("KT", 0);
        if (entry.length() - 2 == pos) {
            auto windData = entry.substr(0, entry.length() - 2);

            types::WindData information;

            information.variable = std::string::npos != windData.rfind("VRB");
            information.direction = static_cast<float>(std::atoi(windData.substr(0, 3).c_str())) * types::degree;
            information.speed = static_cast<float>(std::atoi(windData.substr(3, 5).c_str())) * types::knot;

            if (std::string::npos != windData.find("G"))
                information.gusts = static_cast<float>(std::atoi(windData.substr(6, 8).c_str())) * types::knot;

            configuration.windInformation[station] = information;
            break;
        }
    }

    system::ConfigurationRegistry::instance().setRuntimeConfiguration(configuration);
}

void PlugIn::pdcMessageReceived() {
    PlaySoundA(this->m_pdcNotificationSound.c_str(), NULL, SND_ASYNC);
}

void PlugIn::removeRadarScreen(RadarScreen* screen) {
    auto it = std::find(this->m_screens.begin(), this->m_screens.end(), screen);
    if (this->m_screens.end() != it) {
        this->m_screens.erase(it);
        delete screen;
    }
}
