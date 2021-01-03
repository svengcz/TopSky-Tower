/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * License:
 *   LGPLv3
 * Brief:
 *   Implements the EuroScope plug-in definition
 */

#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"

#include <fstream>
#include <Windows.h>

#define CURL_STATICLIB 1
#include <curl/curl.h>

#include <helper/String.h>
#include <surveillance/ConfigurationRegistry.h>
#include <surveillance/PdcControl.h>
#include <version.h>

#include "ui/PdcDepartureClearanceWindow.h"
#include "ui/PdcMessageViewerWindow.h"
#include "PlugIn.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

using namespace topskytower;
using namespace topskytower::euroscope;

PlugIn::PlugIn() :
        EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
                                 PLUGIN_NAME,
                                 PLUGIN_VERSION_BUILD,
                                 PLUGIN_DEVELOPER,
                                 PLUGIN_COPYRIGHT),
        m_settingsPath(),
        m_screens(),
        m_uiCallback() {
    curl_global_init(CURL_GLOBAL_ALL);

    char path[MAX_PATH] = { 0 };
    GetModuleFileNameA((HINSTANCE)&__ImageBase, path, _countof(path));
    PathRemoveFileSpecA(path);
    this->m_settingsPath = path;

    surveillance::ConfigurationRegistry::instance().configure(this->m_settingsPath);

    this->RegisterTagItemType("Handoff frequency", static_cast<int>(PlugIn::TagItemElement::HandoffFrequency));
    this->RegisterTagItemType("Manually alerts 0", static_cast<int>(PlugIn::TagItemElement::ManuallyAlerts0));
    this->RegisterTagItemType("Manually alerts 1", static_cast<int>(PlugIn::TagItemElement::ManuallyAlerts1));
    this->RegisterTagItemType("Manually alerts 2", static_cast<int>(PlugIn::TagItemElement::ManuallyAlerts2));
    this->RegisterTagItemType("Flight marker", static_cast<int>(PlugIn::TagItemElement::FlightMarker));
    this->RegisterTagItemType("PDC indicator", static_cast<int>(PlugIn::TagItemElement::PdcIndicator));
    this->RegisterTagItemType("SID step climb indicator", static_cast<int>(PlugIn::TagItemElement::SIDStepClimbIndicator));

    this->RegisterTagItemFunction("Menu bar", static_cast<int>(PlugIn::TagItemFunction::AircraftControlMenuBar));
    this->RegisterTagItemFunction("PDC menu bar", static_cast<int>(PlugIn::TagItemFunction::PdcMenu));
}

PlugIn::~PlugIn() {
    for (auto& screen : this->m_screens)
        delete screen;
    this->m_screens.clear();

    curl_global_cleanup();
}

const std::string& PlugIn::settingsPath() const {
    return this->m_settingsPath;
}

EuroScopePlugIn::CRadarScreen* PlugIn::OnRadarScreenCreated(const char* displayName, bool needsRadarContent, bool geoReferenced,
                                                            bool canBeSaved, bool canBeCreated) {
    (void)needsRadarContent;
    (void)geoReferenced;
    (void)canBeSaved;
    (void)canBeCreated;
    (void)displayName;

    this->m_screens.push_back(new RadarScreen(0 == this->m_screens.size()));
    return this->m_screens.back();
}

void PlugIn::OnAirportRunwayActivityChanged() { }

bool PlugIn::visualizeManuallyAlerts(const types::Flight& flight, int idx, char itemString[16]) {
    int localOffset = 0, localIdx = 0;
    bool inserted = false;

    if (true == flight.onMissedApproach()) {
        localOffset += 9;
        if (localIdx == idx) {
            std::strcat(itemString, "MIS/APP ");
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

void PlugIn::OnGetTagItem(EuroScopePlugIn::CFlightPlan flightPlan, EuroScopePlugIn::CRadarTarget radarTarget,
                          int itemCode, int tagData, char itemString[16], int* colorCode, COLORREF* rgb,
                          double* fontSize) {
    (void)flightPlan;
    (void)tagData;
    (void)rgb;
    (void)fontSize;

    /* do not handle invalid radar targets */
    if (false == radarTarget.IsValid())
        return;

    /* initialize default values */
    std::string callsign(radarTarget.GetCallsign());
    itemString[0] = '\0';
    *colorCode = EuroScopePlugIn::TAG_COLOR_DEFAULT;

    const auto& flight = surveillance::FlightRegistry::instance().flight(callsign);

    switch (static_cast<PlugIn::TagItemElement>(itemCode)) {
    case PlugIn::TagItemElement::HandoffFrequency:
        /* test all loaded screens */
        for (const auto& screen : std::as_const(this->m_screens)) {
            /* found the correct screen with the handoff */
            if (true == screen->sectorControl().handoffRequired(callsign)) {
                auto& info = screen->sectorControl().handoffSector(callsign);
                std::string msg = info.identifier() + " " + info.primaryFrequency();

                std::strncpy(itemString, msg.c_str(), 16 < (msg.length() + 1) ? 16 : msg.length() + 1);
                *colorCode = EuroScopePlugIn::TAG_COLOR_NOTIFIED;
                break;
            }
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

        if (true == surveillance::PdcControl::instance().messagesAvailable(flight))
            *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
        else
            *colorCode = EuroScopePlugIn::TAG_COLOR_NON_CONCERNED;

        break;
    }
    case PlugIn::TagItemElement::SIDStepClimbIndicator:
        if (flight.flightPlan().type() == types::FlightPlan::Type::IFR) {
            const auto& config = surveillance::ConfigurationRegistry::instance().airportConfiguration(flight.flightPlan().origin());
            auto sidIt = config.sids.find(flight.flightPlan().departureRoute());

            if (config.sids.cend() != sidIt) {
                if (true == sidIt->second.containsStepClimbs) {
                    std::strcpy(itemString, "YES");
                    *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
                }
                else {
                    std::strcpy(itemString, "NO");
                    *colorCode = EuroScopePlugIn::TAG_COLOR_NON_CONCERNED;
                }
            }
            else {
                std::strcpy(itemString, "UKN");
                *colorCode = EuroScopePlugIn::TAG_COLOR_EMERGENCY;
            }
        }
        else {
            std::strcpy(itemString, "VFR");
            *colorCode = EuroScopePlugIn::TAG_COLOR_NON_CONCERNED;
        }
        break;
    default:
        break;
    }
}

void PlugIn::handleHandoffPerform(POINT point, RECT area, const std::string& callsign, bool tracked) {
    auto radarTarget = this->RadarTargetSelectASEL();

    /* test all loaded screens */
    for (const auto& screen : std::as_const(this->m_screens)) {
        /* found the correct screen with the handoff */
        if (true == screen->sectorControl().handoffRequired(callsign)) {
            auto controllers = screen->sectorControl().handoffStations(callsign);

            /* check if handoff or a release is needed */
            if (1 == controllers.size()) {
                /* handoff to unicom */
                if (true == tracked) {
                    if (0 == controllers.front().size())
                        radarTarget.GetCorrelatedFlightPlan().EndTracking();
                    else
                        radarTarget.GetCorrelatedFlightPlan().InitiateHandoff(controllers.front().c_str());
                }
                screen->sectorControl().handoffPerformed(callsign);
            }
            else {
                RadarScreen::EuroscopeEvent eventEntry = {
                    static_cast<int>(PlugIn::TagItemFunction::HandoffControllerSelectEvent),
                    callsign,
                    "",
                    point,
                    area
                };
                screen->registerEuroscopeEvent(std::move(eventEntry));
            }

            return;
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

void PlugIn::updateFlightStrip(EuroScopePlugIn::CRadarTarget& radarTarget, int idx, const std::string& marker) {
    if (8 < idx)
        return;

    std::string stripEntry(radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().GetFlightStripAnnotation(idx));

    std::size_t pos = stripEntry.find(marker);
    if (std::string::npos != pos)
        stripEntry.erase(pos, marker.length());
    else
        stripEntry += marker;

    radarTarget.GetCorrelatedFlightPlan().GetControllerAssignedData().SetFlightStripAnnotation(idx, stripEntry.c_str());
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

void PlugIn::OnFunctionCall(int functionId, const char* itemString, POINT pt, RECT area) {
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
    bool tracked = radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe();
    auto& flight = surveillance::FlightRegistry::instance().flight(callsign);

    /* check if an handoff is possible */
    RadarScreen* flightScreen = nullptr;
    for (const auto& screen : std::as_const(this->m_screens)) {
        if (true == screen->sectorControl().isInSector(flight)) {
            flightScreen = screen;
            break;
        }
    }

    /* nothing to do */
    if (nullptr == flightScreen)
        return;

    switch (static_cast<PlugIn::TagItemFunction>(functionId)) {
    case PlugIn::TagItemFunction::AircraftControlMenuBar:
        this->OpenPopupList(area, "Aircraft menu", 2);

        /* define the menu bar for the tracking functions */
        if (true == radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe()) {
            this->AddPopupListElement("Transfer", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                      false == flightScreen->sectorControl().handoffRequired(flight.callsign()), false);
            this->AddPopupListElement("Man. Transfer", "", static_cast<int>(PlugIn::TagItemFunction::HandoffSectorChangeEvent),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                      false == flightScreen->sectorControl().handoffPossible(flight.callsign()), false);
            this->AddPopupListElement("Release", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform));
            this->AddPopupListElement("CTR change", "", static_cast<int>(PlugIn::TagItemFunction::SectorControllerHandover),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                      false == flightScreen->sectorControl().sectorHandoverPossible() || false == tracked, false);
        }
        else {
            std::string_view ctrCallsign = radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerCallsign();
            std::string_view trackedCallsign = radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerCallsign();

            this->AddPopupListElement("Assume", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, 0 != trackedCallsign.length(), false);
            this->AddPopupListElement("Accept", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                      this->ControllerMyself().GetCallsign() != ctrCallsign, false);
            this->AddPopupListElement("Refuse", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                      this->ControllerMyself().GetCallsign() != ctrCallsign, false);
            this->AddPopupListElement("Handoff", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform),
                                      false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX,
                                      false == flightScreen->sectorControl().handoffRequired(flight.callsign()), false);
        }
        this->AddPopupListElement("-------------", "-", 0, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true, false);

        /* define the menu bar for the manually alerts */
        if (true == radarTarget.GetCorrelatedFlightPlan().IsValid()) {
            this->AddPopupListElement("Mis/App", (true == flight.onMissedApproach() ? "X" : ""),
                                      static_cast<int>(PlugIn::TagItemFunction::AircraftControlSignal));
            this->AddPopupListElement("Irreg", (true == flight.irregularHandoff() ? "X" : ""),
                                      static_cast<int>(PlugIn::TagItemFunction::AircraftControlSignal));
            this->AddPopupListElement("Est", (true == flight.establishedOnILS() ? "X" : ""),
                                      static_cast<int>(PlugIn::TagItemFunction::AircraftControlSignal));
            this->AddPopupListElement("Mark", (true == flight.markedByController() ? "X" : ""),
                                      static_cast<int>(PlugIn::TagItemFunction::AircraftControlSignal));
        }

        break;
    case PlugIn::TagItemFunction::AircraftControlSignal:
        if (0 == std::strncmp(itemString, "Mis/App", 7))
            PlugIn::updateManuallyAlerts(radarTarget, "MISAP_");
        else if (0 == std::strncmp(itemString, "Irreg", 5))
            PlugIn::updateManuallyAlerts(radarTarget, "IRREG_");
        else if (0 == std::strncmp(itemString, "Est", 3))
            PlugIn::updateManuallyAlerts(radarTarget, "EST_");
        else if (0 == std::strncmp(itemString, "Mark", 4))
            PlugIn::updateFlightStrip(radarTarget, 7, "K");
        break;
    case PlugIn::TagItemFunction::HandoffPerform:
        if (0 == std::strncmp(itemString, "Release", 7))
            radarTarget.GetCorrelatedFlightPlan().EndTracking();
        else if (0 == std::strncmp(itemString, "Assume", 6))
            radarTarget.GetCorrelatedFlightPlan().StartTracking();
        else if (0 == std::strncmp(itemString, "Accept", 6))
            radarTarget.GetCorrelatedFlightPlan().AcceptHandoff();
        else if (0 == std::strncmp(itemString, "Refuse", 6))
            radarTarget.GetCorrelatedFlightPlan().RefuseHandoff();
        else
            this->handleHandoffPerform(pt, area, callsign, tracked);
        break;
    case PlugIn::TagItemFunction::HandoffControllerSelectEvent:
        if (true == flightScreen->sectorControl().handoffRequired(callsign)) {
            auto controllers = flightScreen->sectorControl().handoffStations(callsign);
            auto& info = flightScreen->sectorControl().handoffSector(callsign);
            this->OpenPopupList(area, "Handoff To", 2);

            for (const auto& controller : std::as_const(controllers)) {
                this->AddPopupListElement(controller.c_str(), info.primaryFrequency().c_str(),
                    static_cast<int>(PlugIn::TagItemFunction::HandoffControllerSelect));
            }
        }
        break;
    case PlugIn::TagItemFunction::HandoffControllerSelect:
        if (true == flightScreen->sectorControl().handoffRequired(callsign)) {
            if (true == tracked)
                radarTarget.GetCorrelatedFlightPlan().InitiateHandoff(itemString);
            flightScreen->sectorControl().handoffPerformed(callsign);
        }
        break;
    case PlugIn::TagItemFunction::HandoffSectorChangeEvent:
        if (true == flightScreen->sectorControl().handoffRequired(callsign) || true == flightScreen->sectorControl().handoffPossible(callsign)) {
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
        if (true == flightScreen->sectorControl().handoffRequired(callsign) || true == flightScreen->sectorControl().handoffPossible(callsign)) {
            auto sectors = flightScreen->sectorControl().handoffSectors();

            this->OpenPopupList(area, "Handoff sectors", 2);
            for (const auto& sector : std::as_const(sectors)) {
                this->AddPopupListElement(sector.identifier().c_str(), sector.primaryFrequency().c_str(),
                    static_cast<int>(PlugIn::TagItemFunction::HandoffSectorSelect));
            }
        }
        break;
    case PlugIn::TagItemFunction::HandoffSectorSelect:
        if (true == flightScreen->sectorControl().handoffRequired(callsign) || true == flightScreen->sectorControl().handoffPossible(callsign))
            flightScreen->sectorControl().handoffSectorSelect(callsign, itemString);
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
                /* TODO */
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
        this->OpenPopupList(area, "PDC", 1);
        this->AddPopupListElement("Read", "", static_cast<int>(PlugIn::TagItemFunction::PdcReadMessage), false,
                                  2, false == surveillance::PdcControl::instance().messagesAvailable(flight), false);
        this->AddPopupListElement("Send stand-by", "", static_cast<int>(PlugIn::TagItemFunction::PdcSendStandby));
        this->AddPopupListElement("Send clearance", "", static_cast<int>(PlugIn::TagItemFunction::PdcSendClearance),
                                  false, 2, true == flight.flightPlan().clearedFlag(), false);
        break;
    case PlugIn::TagItemFunction::PdcReadMessage:
    {
        auto message = surveillance::PdcControl::instance().nextMessage(flight);
        auto screen = this->findLastActiveScreen();

        auto viewer = new PdcMessageViewerWindow(screen, message);
        viewer->setActive(true);

        break;
    }
    case PlugIn::TagItemFunction::PdcSendStandby:
        surveillance::PdcControl::instance().sendStandbyMessage(flight);
        break;
    case PlugIn::TagItemFunction::PdcSendClearance:
    {
        auto screen = this->findLastActiveScreen();

        surveillance::PdcControl::ClearanceMessagePtr message(new surveillance::PdcControl::ClearanceMessage());
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
    default:
        break;
    }
}
