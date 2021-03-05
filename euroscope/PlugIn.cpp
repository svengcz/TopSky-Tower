/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the EuroScope plug-in definition
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <filesystem>
#include <fstream>
#include <regex>

#include <Windows.h>
#include <shlwapi.h>

#define CURL_STATICLIB 1
#include <curl/curl.h>

#include <helper/String.h>
#include <management/NotamControl.h>
#include <management/PdcControl.h>
#include <surveillance/FlightPlanControl.h>
#include <surveillance/RadioControl.h>
#include <system/ConfigurationRegistry.h>
#include <version.h>

#include "ui/MessageViewerWindow.h"
#include "ui/PdcDepartureClearanceWindow.h"
#include "ui/PdcMessageViewerWindow.h"
#include "Converter.h"
#include "PlugIn.h"
#include "VersionChecker.h"

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
        m_pdcNotificationSound(),
        m_windowClass{
            NULL,
            HiddenWindow,
            NULL,
            NULL,
            GetModuleHandle(NULL),
            NULL,
            NULL,
            NULL,
            NULL,
            "RDFHiddenWindowClass"
        },
        m_hiddenWindow(nullptr),
        m_transmissions(),
        m_ipc(this) {
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
    this->RegisterTagItemType("Holding point", static_cast<int>(PlugIn::TagItemElement::HoldingPoint));

    this->RegisterTagItemFunction("Menu bar", static_cast<int>(PlugIn::TagItemFunction::AircraftControlMenuBar));
    this->RegisterTagItemFunction("PDC menu bar", static_cast<int>(PlugIn::TagItemFunction::PdcMenu));
    this->RegisterTagItemFunction("FP check menu", static_cast<int>(PlugIn::TagItemFunction::FlightPlanCheckMenu));
    this->RegisterTagItemFunction("Stand menu", static_cast<int>(PlugIn::TagItemFunction::StandControlMenu));
    this->RegisterTagItemFunction("Departure Ground status menu", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusMenu));
    this->RegisterTagItemFunction("Arrival Ground status menu", static_cast<int>(PlugIn::TagItemFunction::ArrivalGroundStatusMenu));
    this->RegisterTagItemFunction("Surveillance visualization trigger", static_cast<int>(PlugIn::TagItemFunction::SurveillanceAlertVisualization));
    this->RegisterTagItemFunction("Draw departure route (auto hide)", static_cast<int>(PlugIn::TagItemFunction::DepartureRouteDrawTimeBased));
    this->RegisterTagItemFunction("Draw departure route", static_cast<int>(PlugIn::TagItemFunction::DepartureRouteDraw));
    this->RegisterTagItemFunction("Holding point menu", static_cast<int>(PlugIn::TagItemFunction::HoldingPointCandidatesMenu));

    /* search for the sound file and register the PDC sound callback */
    for (auto& entry : fs::recursive_directory_iterator(path)) {
        if (true == fs::is_regular_file(entry) && "TopSkyTower-Pdc.wav" == entry.path().filename().string()) {
            this->m_pdcNotificationSound = entry.path().string();
            management::PdcControl::instance().registerNotificationCallback(this, &PlugIn::pdcMessageReceived);
            break;
        }
    }

    const auto& systemConfig = system::ConfigurationRegistry::instance().systemConfiguration();
    if (true == systemConfig.valid && true == systemConfig.rdfActive && false == this->m_ipc.isSlave()) {
        /* register the window class and create the window */
        RegisterClassA(&this->m_windowClass);
        this->m_hiddenWindow = CreateWindowA("RDFHiddenWindowClass", "RDFHiddenWindow", NULL, 0, 0, 0, 0, NULL, NULL,
                                             GetModuleHandle(NULL), reinterpret_cast<LPVOID>(this));

        if (S_OK != GetLastError())
            this->DisplayUserMessage("Message", PLUGIN_NAME, "Unable to open RDF communication", true, true, true, false, false);
    }

    management::NotamControl::instance().registerNotificationCallback(this, &PlugIn::checkNotamsOfActiveRunways);
}

PlugIn::~PlugIn() {
    management::NotamControl::instance().deleteNotificationCallback(this);

    this->m_screens.clear();

    if (false == this->m_errorMode) {
        Gdiplus::GdiplusShutdown(__gdiplusToken);
        curl_global_cleanup();
    }

    if (NULL != this->m_hiddenWindow)
        DestroyWindow(this->m_hiddenWindow);
    this->m_hiddenWindow = NULL;
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

void PlugIn::checkNotamsOfActiveRunways() {
    /* find the current screen */
    auto screen = this->findLastActiveScreen(false);
    if (nullptr == screen)
        return;

    const auto& config = system::ConfigurationRegistry::instance().runtimeConfiguration();

    /* check if runway NOTAM violates the active runways */
    for (const auto& airportNotams : std::as_const(management::NotamControl::instance().notams())) {
        auto depRunways = config.activeDepartureRunways.find(airportNotams.first);
        auto arrRunways = config.activeArrivalRunways.find(airportNotams.first);

        /* aiport of NOTAM not active */
        if (config.activeDepartureRunways.cend() == depRunways && config.activeArrivalRunways.cend() == arrRunways)
            continue;

        /* test all NOTAMs */
        for (const auto& notam : std::as_const(airportNotams.second)) {
            /* ignore irrelevant, non-parsed NOTAMs or inactive */
            if (management::NotamCategory::Runway != notam->category)
                continue;
            if (management::NotamInterpreterState::Success != notam->interpreterState)
                continue;
            if (false == notam->isActive())
                continue;

            /* check the runways */
            const auto& runways = static_cast<management::RunwayNotam*>(notam.get())->sections;
            for (const auto& runway : std::as_const(runways)) {
                auto dIt = std::find(depRunways->second.cbegin(), depRunways->second.cend(), runway);
                auto aIt = std::find(arrRunways->second.cbegin(), arrRunways->second.cend(), runway);

                if (depRunways->second.cend() != dIt || arrRunways->second.cend() != aIt) {
                    auto viewer = new MessageViewerWindow(screen, "NOTAM violation",
                        "Active runway " + runway + " is deactivated by NOTAM " + notam->title);
                    viewer->setActive(true);
                    return;
                }
            }
        }
    }
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

    static bool firstCall = true;

    if (false == this->m_errorMode) {
        this->m_screens.push_back(new RadarScreen());

        if (1 == this->m_screens.size())
            this->OnAirportRunwayActivityChanged();

        if (true == firstCall) {
            VersionChecker::checkForUpdates(this->m_screens.back());
            firstCall = false;
        }

        return this->m_screens.back();
    }

    return nullptr;
}

bool PlugIn::visualizeManuallyAlerts(const types::Flight& flight, int idx, char itemString[16]) {
    int localOffset = 0, localIdx = 0;
    bool inserted = false;

    if (true == flight.onMissedApproach()) {
        localOffset += 5;
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

    if (true == flight.readyForDeparture()) {
        localOffset += 5;

        if (16 <= localOffset) {
            localIdx += localOffset / 16;
            localOffset = 0;
            if (idx < localIdx)
                return inserted;
        }

        if (localIdx == idx) {
            std::strcat(itemString, "RDY ");
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
        case surveillance::FlightPlanControl::ErrorCode::Event:
            code = "EVT";
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

std::string PlugIn::findScratchPadEntry(const EuroScopePlugIn::CFlightPlan& plan, const std::string& marker, const std::string& entry) {
    /* check if the scratch pad contains a new message */
    std::string scratchpad = plan.GetControllerAssignedData().GetScratchPadString();
    std::string identifier = marker + "/" + entry + "/";
    std::size_t pos = scratchpad.find(identifier);

    if (std::string::npos != pos) {
        auto retval = scratchpad.substr(pos + identifier.length());
        scratchpad.erase(pos, scratchpad.length());
        plan.GetControllerAssignedData().SetScratchPadString(scratchpad.c_str());
        return retval;
    }

    return "";
}

void PlugIn::updateStand(const types::Flight& flight, EuroScopePlugIn::CFlightPlan& plan) {
    auto stand = PlugIn::findScratchPadEntry(plan, "GRP", "S");

    if (0 != stand.length()) {
        std::string annotation = "s/" + stand + "/s";
        plan.GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand), annotation.c_str());

        for (auto& screen : this->m_screens) {
            if (false == screen->isInitialized())
                continue;

            auto type = screen->identifyType(flight);

            if (types::Flight::Type::Unknown != type)
                screen->standControl().assignManually(flight, type, stand);
        }
    }
}

void PlugIn::updateHoldingPoint(const types::Flight& flight, EuroScopePlugIn::CFlightPlan& plan) {
    auto holdingPoint = PlugIn::findScratchPadEntry(plan, "TST", "HP");

    if (0 != holdingPoint.length()) {
        for (auto& screen : this->m_screens) {
            if (false == screen->isInitialized())
                continue;

            auto type = screen->identifyType(flight);

            if (types::Flight::Type::Unknown != type)
                screen->departureSequenceControl().setHoldingPoint(flight, holdingPoint);
        }
    }
}

void PlugIn::updateSectorHandoff(const types::Flight& flight) {
    for (auto& screen : this->m_screens) {
        if (true == screen->sectorControl().handoffRequired(flight))
            screen->sectorControl().handoffPerformed(flight);
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
    std::string callsign(radarTarget.GetCallsign());
    if (true == this->m_errorMode || false == radarTarget.IsValid() || false == system::FlightRegistry::instance().flightExists(callsign))
        return;

    /* initialize default values */
    itemString[0] = '\0';
    *colorCode = EuroScopePlugIn::TAG_COLOR_DEFAULT;

    /* get the screen and the converted flight information */
    const types::Flight& flight = system::FlightRegistry::instance().flight(callsign);
    auto flightScreen = this->findLastActiveScreen(true);
    if (nullptr == flightScreen)
        return;

    if (true == flightScreen->sectorControl().handoffRequired(flight)) {
        auto entry  = radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Handoff));

        if (nullptr != entry && 'H' == entry[0])
            this->updateSectorHandoff(flight);
    }

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
        if (false == flightScreen->sectorControl().isInOwnSector(flight, flightScreen->identifyType(flight)) && false == flight.isTracked())
            finalizeRoute = false;

        if (true == finalizeRoute && types::FlightPlan::Type::VFR != flight.flightPlan().type()) {
            std::string departure = flight.flightPlan().departureRoute() + "/";
            departure += radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().GetDepartureRwy();
            std::string route(radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().GetRoute());

            /* check if the route is already configured */
            if (std::string::npos == route.find(departure, 0)) {
                auto newRoute = departure + " " + flight.flightPlan().textRoute();

                /* write into the flight plan */
                radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().SetRoute(newRoute.c_str());
                radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().AmendFlightPlan();
            }

            /* update the initial clearance limit */
            auto& config = system::ConfigurationRegistry::instance().airportConfiguration(flight.flightPlan().origin());
            auto sidIt = config.sids.find(flight.flightPlan().departureRoute());

            if (config.sids.cend() != sidIt) {
                auto esPlan = radarTarget.GetCorrelatedFlightPlan();
                auto clearedFL = static_cast<int>(sidIt->second.clearanceLimit.convert(types::feet));
                if (esPlan.GetControllerAssignedData().GetClearedAltitude() != clearedFL)
                    esPlan.GetControllerAssignedData().SetClearedAltitude(clearedFL);
            }
        }

        break;
    }
    case PlugIn::TagItemElement::AircraftStand:
    {
        auto stand = flightScreen->standControl().stand(flight);
        if (0 != stand.length()) {
            /* validate that no other controller published a stand already */
            std::strcpy(itemString, stand.c_str());

            /* define the color */
            if (true == flightScreen->standControl().standIsBlocked(stand)) {
                *colorCode = EuroScopePlugIn::TAG_COLOR_EMERGENCY;
            }
            else if (types::Flight::Type::Arrival == flightScreen->identifyType(flight)) {
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
    {
        *colorCode = EuroScopePlugIn::TAG_COLOR_EMERGENCY;
        itemString[0] = '\0';

        bool inSector = flightScreen->sectorControl().isInOwnSector(flight, flightScreen->identifyType(flight));
        if (true == inSector || true == flight.isTracked()) {
            if (true == flightScreen->stcdControl().ntzViolation(flight))
                std::strcat(itemString, "NTZ");
            else if (true == flightScreen->stcdControl().separationLoss(flight))
                std::strcat(itemString, "STC");
            else if (true == flightScreen->ariwsControl().runwayIncursionWarning(flight))
                std::strcat(itemString, "RIW");
            else if (true == flightScreen->cmacControl().conformanceMonitoringAlert(flight, flightScreen->identifyType(flight)))
                std::strcat(itemString, "CMA");
            else if (true == flightScreen->mtcdControl().conflictsExist(flight))
                std::strcat(itemString, "MTC");
        }

        break;
    }
    case PlugIn::TagItemElement::HoldingPoint:
        if (true == flightScreen->departureSequenceControl().hasHoldingPoint(flight)) {
            auto& point = flightScreen->departureSequenceControl().holdingPoint(flight);
            std::strcat(itemString, point.name.c_str());
        }
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

            /* update all screens */
            this->updateSectorHandoff(flight);
            radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Handoff), "H");
        }
        else if (0 != controllers.size()) {
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

RadarScreen* PlugIn::findLastActiveScreen(bool needsInitialization) {
    std::chrono::system_clock::time_point newest;
    RadarScreen* retval = nullptr;

    for (auto& screen : this->m_screens) {
        if (screen->lastRenderingTime() >= newest && (false == needsInitialization || true == screen->isInitialized())) {
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
            case surveillance::FlightPlanControl::ErrorCode::Event:
                retval += "Invalid event route filed!\n";
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
                                const types::Flight& flight, bool arrival) {
    std::string scratchPadExtend;
    bool overwrite = false;
    std::uint16_t mask = 0;

    if (false == arrival) {
        mask |= static_cast<std::uint16_t>(flight.flightPlan().arrivalFlag());

        if ("CLEAR" == view) {
            scratchPadExtend = "ST-UP";
            overwrite = true;
        }
        else if ("ST-UP" == view) {
            mask |= static_cast<std::uint16_t>(types::FlightPlan::AtcCommand::StartUp);
            scratchPadExtend = "ST-UP";
            overwrite = true;
        }
        else if ("PUSH" == view) {
            mask |= static_cast<std::uint16_t>(types::FlightPlan::AtcCommand::Pushback);
            scratchPadExtend = "PUSH";
            overwrite = true;
        }
        else if ("TAXI" == view) {
            mask |= static_cast<std::uint16_t>(types::FlightPlan::AtcCommand::TaxiOut);
            scratchPadExtend = "TAXI";
            overwrite = true;
        }
        else if ("DEICE" == view) {
            mask |= static_cast<std::uint16_t>(types::FlightPlan::AtcCommand::Deicing);
            scratchPadExtend = "DE-ICE";
        }
        else if ("LI-UP" == view) {
            mask |= static_cast<std::uint16_t>(types::FlightPlan::AtcCommand::LineUp);
            scratchPadExtend = "TAXI;LINEUP";
            overwrite = true;
        }
        else if ("DEPA" == view) {
            mask |= static_cast<std::uint16_t>(types::FlightPlan::AtcCommand::Departure);
            scratchPadExtend = "DEPA";
            overwrite = true;
        }
    }
    else {
        mask |= static_cast<std::uint16_t>(flight.flightPlan().departureFlag());

        if ("CLEAR" == view) {
            if (true == flight.onMissedApproach())
                PlugIn::updateManuallyAlerts(target, "MISAP_");
        }
        else if ("APPR" == view) {
            mask |= static_cast<std::uint16_t>(types::FlightPlan::AtcCommand::Approach);
            scratchPadExtend = "APPROACH";
            if (true == flight.onMissedApproach())
                PlugIn::updateManuallyAlerts(target, "MISAP_");
        }
        else if ("LAND" == view) {
            mask |= static_cast<std::uint16_t>(types::FlightPlan::AtcCommand::Land);
            scratchPadExtend = "LANDING";
            if (true == flight.onMissedApproach())
                PlugIn::updateManuallyAlerts(target, "MISAP_");
        }
        else if ("TAXI" == view) {
            mask |= static_cast<std::uint16_t>(types::FlightPlan::AtcCommand::TaxiIn);
            scratchPadExtend = "TAXIIN";
            if (true == flight.onMissedApproach())
                PlugIn::updateManuallyAlerts(target, "MISAP_");
        }
        else if ("GO-AR" == view) {
            mask |= static_cast<std::uint16_t>(types::FlightPlan::AtcCommand::GoAround);
            scratchPadExtend = "GOAROUND";
            if (false == flight.onMissedApproach())
                PlugIn::updateManuallyAlerts(target, "MISAP_");
        }
    }

    system::FlightRegistry::instance().setAtcClearanceFlag(flight, mask);

    if (0 != scratchPadExtend.length()) {
        auto split = helper::String::splitString(scratchPadExtend, ";");
        for (const auto& entry : std::as_const(split)) {
            std::string scratchpad;

            if (false == overwrite) {
                scratchpad = target.GetCorrelatedFlightPlan().GetControllerAssignedData().GetScratchPadString();
                scratchpad += entry;
            }
            else {
                scratchpad = entry;
            }

            target.GetCorrelatedFlightPlan().GetControllerAssignedData().SetScratchPadString(scratchpad.c_str());
        }
    }
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
        case PlugIn::TagItemFunction::UiDropDownRequest:
        {
            this->OpenPopupList(area, "", 1);
            auto split = helper::String::splitString(itemString, ";");
            for (const auto& element : std::as_const(split)) {
                if (0 != element.length())
                    this->AddPopupListElement(element.c_str(), "", static_cast<int>(PlugIn::TagItemFunction::UiDropDownResponse));
            }
            break;
        }
        case PlugIn::TagItemFunction::UiDropDownResponse:
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
    std::string callsign(radarTarget.GetCallsign());
    if (false == radarTarget.IsValid() || false == system::FlightRegistry::instance().flightExists(callsign))
        return;

    /* check if we are tracking and search the flight */
    const types::Flight& flight = system::FlightRegistry::instance().flight(callsign);
    auto flightScreen = this->findLastActiveScreen(true);

    /* nothing to do */
    if (nullptr == flightScreen)
        return;
    /* disable the screen selection */
    flightScreen->activateStandOnScreenSelection(false, flight.callsign());

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
                                      false == flightScreen->sectorControl().handoffPossible(flight, flightScreen->identifyType(flight)), false);
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
            this->AddPopupListElement("Ready", (true == flight.readyForDeparture() ? "X" : ""),
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
            if (0 == std::strncmp(itemString, "M/A", 3)) {
                PlugIn::updateManuallyAlerts(radarTarget, "MISAP_");
            }
            else if (0 == std::strncmp(itemString, "Ready", 5)) {
                PlugIn::updateManuallyAlerts(radarTarget, "RDY_");
            }
            else if (0 == std::strncmp(itemString, "Irreg", 5)) {
                PlugIn::updateManuallyAlerts(radarTarget, "IRREG_");
            }
            else if (0 == std::strncmp(itemString, "Est", 3)) {
                PlugIn::updateManuallyAlerts(radarTarget, "EST_");
            }
            else if (0 == std::strncmp(itemString, "Mark", 4)) {
                if (true == flight.markedByController())
                    radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Marker), "");
                else
                    radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Marker), "K");
                this->OnRadarTargetPositionUpdate(radarTarget);
            }
        }
        break;
    case PlugIn::TagItemFunction::HandoffPerform:
        if (0 == std::strncmp(itemString, "Release", 7)) {
            radarTarget.GetCorrelatedFlightPlan().EndTracking();

            /* check if a handoff to UNICOM is ongoing */
            if (true == flightScreen->sectorControl().handoffRequired(flight)) {
                auto controllers = flightScreen->sectorControl().handoffStations(flight);
                if (0 == controllers.front().size()) {
                    this->updateSectorHandoff(flight);
                    radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Handoff), "H");
                }
            }
        }
        else if (0 == std::strncmp(itemString, "Assume", 6)) {
            radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Handoff), "");
            radarTarget.GetCorrelatedFlightPlan().StartTracking();
            this->OnRadarTargetPositionUpdate(radarTarget);
        }
        else if (0 == std::strncmp(itemString, "Accept", 6)) {
            radarTarget.GetCorrelatedFlightPlan().AcceptHandoff();
            this->OnRadarTargetPositionUpdate(radarTarget);
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

            this->updateSectorHandoff(flight);
            radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Handoff), "H");
        }
        break;
    case PlugIn::TagItemFunction::HandoffSectorChangeEvent:
        if (true == flightScreen->sectorControl().handoffRequired(flight) ||
            true == flightScreen->sectorControl().handoffPossible(flight, flightScreen->identifyType(flight)))
        {
            RadarScreen::EuroscopeEvent eventEntry = {
                static_cast<int>(PlugIn::TagItemFunction::HandoffSectorChange),
                flight.callsign(),
                "",
                pt,
                area
            };
            flightScreen->registerEuroscopeEvent(std::move(eventEntry));
        }
        break;
    case PlugIn::TagItemFunction::HandoffSectorChange:
        if (true == flightScreen->sectorControl().handoffRequired(flight) ||
            true == flightScreen->sectorControl().handoffPossible(flight, flightScreen->identifyType(flight)))
        {
            auto sectors = flightScreen->sectorControl().handoffSectors();

            if (0 != sectors.size()) {
                this->OpenPopupList(area, "Handoff sectors", 2);
                for (const auto& sector : std::as_const(sectors)) {
                    this->AddPopupListElement(sector.identifier().c_str(), sector.primaryFrequency().c_str(),
                                              static_cast<int>(PlugIn::TagItemFunction::HandoffSectorSelect));
                }
            }
        }
        break;
    case PlugIn::TagItemFunction::HandoffSectorSelect:
        if (true == flightScreen->sectorControl().handoffRequired(flight) ||
            true == flightScreen->sectorControl().handoffPossible(flight, flightScreen->identifyType(flight)))
        {
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
                flightScreen->registerEuroscopeEvent(std::move(esEvent));
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
            auto viewer = new PdcMessageViewerWindow(flightScreen, message);
            viewer->setActive(true);
        }
        break;
    }
    case PlugIn::TagItemFunction::PdcSendStandby:
        management::PdcControl::instance().sendStandbyMessage(flight);
        break;
    case PlugIn::TagItemFunction::PdcSendClearance:
    {
        management::PdcControl::ClearanceMessagePtr message(new management::PdcControl::ClearanceMessage());
        message->sender = flight.flightPlan().origin();
        message->receiver = flight.callsign();
        message->destination = flight.flightPlan().destination();
        message->sid = flight.flightPlan().departureRoute();
        message->runway = radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().GetDepartureRwy();
        message->frequency = flightScreen->sectorControl().ownSector().primaryFrequency();
        message->squawk = std::to_string(flight.flightPlan().assignedSquawk());

        if (this->GetTransitionAltitude() <= flight.flightPlan().clearanceLimit().convert(types::feet)) {
            auto limit = static_cast<int>(flight.flightPlan().clearanceLimit().convert(types::feet));
            limit /= 100;
            message->clearanceLimit = "FL" + std::to_string(limit);
        }
        else {
            message->clearanceLimit = std::to_string(static_cast<int>(flight.flightPlan().clearanceLimit().convert(types::feet)));
        }

        auto viewer = new PdcDepartureClearanceWindow(flightScreen, message);
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
        auto viewer = new MessageViewerWindow(flightScreen, "FP check: " + flight.callsign(), messageLog);
        viewer->setActive(true);
        break;
    }
    case PlugIn::TagItemFunction::FlightPlanCheckOverwrite:
        surveillance::FlightPlanControl::instance().overwrite(flight.callsign());
        break;
    case PlugIn::TagItemFunction::StandControlMenu:
    {
        if (false == flight.isTrackedByOther()) {
            this->OpenPopupList(area, "Stand", 1);
            this->AddPopupListElement("Publish", "", static_cast<int>(PlugIn::TagItemFunction::StandControlPublish));
            this->AddPopupListElement("Automatic", "", static_cast<int>(PlugIn::TagItemFunction::StandControlAutomatic));
            this->AddPopupListElement("Screen select", "", static_cast<int>(PlugIn::TagItemFunction::StandControlScreenSelect));
            this->AddPopupListElement("Manual", "", static_cast<int>(PlugIn::TagItemFunction::StandControlManualEvent));
        }
        break;
    }
    case PlugIn::TagItemFunction::StandControlPublish:
    {
        std::string scratchpad = radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetScratchPadString();
        auto stand = "GRP/S/" + flightScreen->standControl().stand(flight);
        scratchpad += stand;
        radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetScratchPadString(scratchpad.c_str());
        break;
    }
    case PlugIn::TagItemFunction::StandControlAutomatic:
        flightScreen->standControl().removeFlight(flight.callsign());
        flightScreen->standControl().updateFlight(flight, flightScreen->identifyType(flight));
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
        auto stands = flightScreen->standControl().allPossibleAndAvailableStands(flight);
        this->OpenPopupList(area, "Stands", 1);
        for (const auto& stand : std::as_const(stands))
            this->AddPopupListElement(stand.c_str(), "", static_cast<int>(PlugIn::TagItemFunction::StandControlManualSelect));
        break;
    }
    case PlugIn::TagItemFunction::StandControlManualSelect:
    {
        std::string scratchpad = radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetScratchPadString();
        auto stand = std::string("GRP/S/") + itemString;
        scratchpad += stand;
        radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand), "");
        radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetScratchPadString(scratchpad.c_str());
        break;
    }
    case PlugIn::TagItemFunction::StandControlScreenSelect:
        flightScreen->activateStandOnScreenSelection(true, flight.callsign());
        break;
    case PlugIn::TagItemFunction::DepartureGroundStatusMenu:
        if (false == flight.isTrackedByOther()) {
            this->OpenPopupList(area, "Status", 1);
            this->AddPopupListElement("CLEAR", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
            this->AddPopupListElement("ST-UP", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
            this->AddPopupListElement("DEICE", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
            this->AddPopupListElement("PUSH", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
            this->AddPopupListElement("TAXI", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
            this->AddPopupListElement("LI-UP", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
            this->AddPopupListElement("DEPA", "", static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusSelect));
        }
        break;
    case PlugIn::TagItemFunction::DepartureGroundStatusSelect:
        this->updateGroundStatus(radarTarget, itemString, flight, false);
        break;
    case PlugIn::TagItemFunction::ArrivalGroundStatusMenu:
        if (false == flight.isTrackedByOther()) {
            this->OpenPopupList(area, "Status", 1);
            this->AddPopupListElement("CLEAR", "", static_cast<int>(PlugIn::TagItemFunction::ArrivalGroundStatusSelect));
            this->AddPopupListElement("APPR", "", static_cast<int>(PlugIn::TagItemFunction::ArrivalGroundStatusSelect));
            this->AddPopupListElement("LAND", "", static_cast<int>(PlugIn::TagItemFunction::ArrivalGroundStatusSelect));
            this->AddPopupListElement("TAXI", "", static_cast<int>(PlugIn::TagItemFunction::ArrivalGroundStatusSelect));
            this->AddPopupListElement("GO-AR", "", static_cast<int>(PlugIn::TagItemFunction::ArrivalGroundStatusSelect));
        }
        break;
    case PlugIn::TagItemFunction::ArrivalGroundStatusSelect:
        this->updateGroundStatus(radarTarget, itemString, flight, true);
        break;
    case PlugIn::TagItemFunction::SurveillanceAlertVisualization:
        flightScreen->activateSurveillanceVisualization(flight.callsign());
        break;
    case PlugIn::TagItemFunction::DepartureRouteDrawTimeBased:
        flightScreen->activateDepartureRouteVisualization(flight.callsign(), system::ConfigurationRegistry::instance().systemConfiguration().surveillanceVisualizationDuration);
        break;
    case PlugIn::TagItemFunction::DepartureRouteDraw:
        flightScreen->activateDepartureRouteVisualization(flight.callsign(), -1.0f * types::second);
        break;
    case PlugIn::TagItemFunction::HoldingPointCandidatesMenu:
        if (false == flight.isTrackedByOther()) {
            auto holdingPoints = flightScreen->departureSequenceControl().holdingPointCandidates(flight);
            if (0 != holdingPoints.size()) {
                this->OpenPopupList(area, "H/P", 1);

                for (const auto& point : std::as_const(holdingPoints))
                    this->AddPopupListElement(point.name.c_str(), "", static_cast<int>(PlugIn::TagItemFunction::HoldingPointCandidatesSelect));
            }
        }
        break;
    case PlugIn::TagItemFunction::HoldingPointCandidatesSelect:
    {
        std::string scratchpad = radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetScratchPadString();
        auto holdingPoint = std::string("TST/HP/") + itemString;
        scratchpad += holdingPoint;
        radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetScratchPadString(scratchpad.c_str());
        break;
    }
    default:
        break;
    }
}

void PlugIn::OnNewMetarReceived(const char* station, const char* fullMetar) {
    auto split = helper::String::splitString(fullMetar, " ");

    /* find the wind entry */
    for (const auto& entry : std::as_const(split)) {
        /* minimum format DDDSSKT, maximum format DDDSSGSSKT */
        if (entry.length() != 7 && entry.length() != 10)
            continue;

        /* find the wind entry */
        auto pos = entry.find("KT", 0);
        if (entry.length() - 2 == pos) {
            auto windData = entry.substr(0, entry.length() - 2);

            types::WindData information;

            information.variable = std::string::npos != windData.rfind("VRB");
            information.direction = static_cast<float>(std::atoi(windData.substr(0, 3).c_str())) * types::degree;
            information.speed = static_cast<float>(std::atoi(windData.substr(3, 5).c_str())) * types::knot;

            if (std::string::npos != windData.find("G") && 10 == entry.length())
                information.gusts = static_cast<float>(std::atoi(windData.substr(6, 8).c_str())) * types::knot;

            system::ConfigurationRegistry::instance().setMetarInformation(station, information);
            break;
        }
    }
}

void PlugIn::OnTimer(int counter) {
    (void)counter;

    std::list<std::string> messages;

    this->m_transmissionsLock.lock();
    messages = std::move(this->m_transmissions);
    this->m_transmissionsLock.unlock();

    for (const auto& message : std::as_const(messages)) {
        auto callsigns = helper::String::splitString(message, ":");
        surveillance::RadioControl::instance().transmissions(callsigns);
    }
}

void PlugIn::OnRadarTargetPositionUpdate(EuroScopePlugIn::CRadarTarget radarTarget) {
    if (false == radarTarget.IsValid())
        return;
    system::FlightRegistry::instance().updateFlight(Converter::convert(radarTarget));
}

void PlugIn::OnFlightPlanFlightPlanDataUpdate(EuroScopePlugIn::CFlightPlan flightPlan) {
    if (false == flightPlan.GetCorrelatedRadarTarget().IsValid())
        return;
    system::FlightRegistry::instance().updateFlight(Converter::convert(flightPlan.GetCorrelatedRadarTarget()));
}

void PlugIn::OnFlightPlanControllerAssignedDataUpdate(EuroScopePlugIn::CFlightPlan flightPlan, int type) {
    /* handle only relevant changes */
    if (EuroScopePlugIn::CTR_DATA_TYPE_TEMPORARY_ALTITUDE != type &&
        EuroScopePlugIn::CTR_DATA_TYPE_SQUAWK != type &&
        EuroScopePlugIn::CTR_DATA_TYPE_SCRATCH_PAD_STRING != type)
    {
        return;
    }

    if (false == flightPlan.GetCorrelatedRadarTarget().IsValid())
        return;

    /* update the internal structures that are effected by the flight plan changes */
    auto flight = Converter::convert(flightPlan.GetCorrelatedRadarTarget());
    system::FlightRegistry::instance().updateFlight(flight);

    this->updateHoldingPoint(flight, flightPlan);
    this->updateStand(flight, flightPlan);
}

void PlugIn::OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan flightPlan) {
    system::FlightRegistry::instance().removeFlight(flightPlan.GetCallsign());
    surveillance::FlightPlanControl::instance().removeFlight(flightPlan.GetCallsign());
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

void PlugIn::afvMessage(const std::string& message) {
    this->m_transmissionsLock.lock();
    this->m_transmissions.push_back(message);
    this->m_transmissionsLock.unlock();
}

RdfIPC& PlugIn::rdfCommunication() {
    return this->m_ipc;
}
