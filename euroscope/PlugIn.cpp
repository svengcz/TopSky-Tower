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

#include <cassert>

#include <helper/String.h>
#include <version.h>

#include "PlugIn.h"

using namespace topskytower::euroscope;

PlugIn::PlugIn() :
        EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
                                 PLUGIN_NAME,
                                 PLUGIN_VERSION_BUILD,
                                 PLUGIN_DEVELOPER,
                                 PLUGIN_COPYRIGHT),
        m_screens() {
    this->RegisterTagItemType("Handoff frequency", static_cast<int>(PlugIn::TagItemElement::HandoffFrequency));
    this->RegisterTagItemType("Manually alerts 0", static_cast<int>(PlugIn::TagItemElement::ManuallyAlerts0));
    this->RegisterTagItemType("Manually alerts 1", static_cast<int>(PlugIn::TagItemElement::ManuallyAlerts1));
    this->RegisterTagItemType("Manually alerts 2", static_cast<int>(PlugIn::TagItemElement::ManuallyAlerts2));

    this->RegisterTagItemFunction("Handoff initiate", static_cast<int>(PlugIn::TagItemFunction::HandoffInitiated));
    this->RegisterTagItemFunction("Handoff sector change", static_cast<int>(PlugIn::TagItemFunction::HandoffSectorChange));
}

PlugIn::~PlugIn() {
    for (auto& screen : this->m_screens)
        delete screen;
    this->m_screens.clear();
}

EuroScopePlugIn::CRadarScreen* PlugIn::OnRadarScreenCreated(const char* displayName, bool needsRadarContent, bool geoReferenced,
                                                            bool canBeSaved, bool canBeCreated) {
    (void)needsRadarContent;
    (void)geoReferenced;
    (void)canBeSaved;
    (void)canBeCreated;
    (void)displayName;

    this->m_screens.push_back(new RadarScreen());
    return this->m_screens.back();
}

void PlugIn::OnAirportRunwayActivityChanged() { }

bool PlugIn::OnCompileCommand(const char* cmdline) {
    (void)cmdline;
    return false;
}

bool PlugIn::visualizeManuallyAlerts(const EuroScopePlugIn::CFlightPlan& flightPlan, int idx, char itemString[16]) const {
    /* invalid flightplan -> no valid messages */
    if (false == flightPlan.IsValid())
        return false;

    /* check if the entries are set */
    std::string scratch = flightPlan.GetControllerAssignedData().GetScratchPadString();
    if (std::string::npos == scratch.find('_'))
        return false;

    auto split = helper::String::splitString(scratch, "_");
    bool inserted = false;

    /* create the messages */
    std::list<std::string> alerts;
    for (const auto& element : std::as_const(split)) {
        std::string message;

        if (element == "MISAP")
            message = "MIS/APP ";
        else if (element == "IRREG")
            message = "IRREG ";
        else if (element == "EST")
            message = "EST ";

        if (0 != message.length() && alerts.end() == std::find(alerts.begin(), alerts.end(), message))
            alerts.push_back(std::move(message));
    }

    /* sort based on priorty */
    alerts.sort([](const std::string& msg0, const std::string& msg1) {
        if ('M' == msg0[0] || 'E' == msg1[0])
            return true;
        else if ('M' == msg1[0] || 'E' == msg0[0])
            return false;
        else
            return false;
    });

    int localOffset = 0, localIdx = 0;
    for (auto it = alerts.cbegin(); alerts.cend() != it && localIdx <= idx;) {
        localOffset += it->length();

        /* next index needed */
        if (16 <= localOffset) {
            localIdx += localOffset / 16;
            localOffset = 0;
        }
        /* insert message */
        else if (idx == localIdx) {
            std::strcat(itemString, it->c_str());
            inserted = true;
        }

        if (0 != localOffset)
            ++it;

        /* skipped the target index */
        if (localIdx > idx)
            break;
    }

    return inserted;
}

void PlugIn::OnGetTagItem(EuroScopePlugIn::CFlightPlan flightPlan, EuroScopePlugIn::CRadarTarget radarTarget,
                          int itemCode, int tagData, char itemString[16], int* colorCode, COLORREF* rgb,
                          double* fontSize) {
    (void)flightPlan;
    (void)fontSize;
    (void)tagData;
    (void)rgb;

    /* do not handle invalid radar targets */
    if (false == radarTarget.IsValid())
        return;

    /* initialize default values */
    std::string callsign(radarTarget.GetCallsign());
    itemString[0] = '\0';
    *colorCode = EuroScopePlugIn::TAG_COLOR_DEFAULT;

    switch (static_cast<PlugIn::TagItemElement>(itemCode)) {
    case PlugIn::TagItemElement::HandoffFrequency:
        /* test all loaded screens */
        for (const auto& screen : std::as_const(this->m_screens)) {
            /* found the correct screen with the handoff */
            if (true == screen->controllerManager().handoffRequired(callsign)) {
                std::strcpy(itemString, screen->controllerManager().handoffFrequency(callsign).c_str());
                *colorCode = EuroScopePlugIn::TAG_COLOR_NOTIFIED;
                break;
            }
        }
        break;
    case PlugIn::TagItemElement::ManuallyAlerts0:
        if (true == this->visualizeManuallyAlerts(radarTarget.GetCorrelatedFlightPlan(), 0, itemString))
            *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
        break;
    case PlugIn::TagItemElement::ManuallyAlerts1:
        if (true == this->visualizeManuallyAlerts(radarTarget.GetCorrelatedFlightPlan(), 1, itemString))
            *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
        break;
    case PlugIn::TagItemElement::ManuallyAlerts2:
        if (true == this->visualizeManuallyAlerts(radarTarget.GetCorrelatedFlightPlan(), 2, itemString))
            *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
        break;
    default:
        break;
    }
}

void PlugIn::handleHandoffPerform(RECT area, const std::string& callsign, bool release, bool tracked) {
    auto radarTarget = this->RadarTargetSelectASEL();

    /* test all loaded screens */
    for (const auto& screen : std::as_const(this->m_screens)) {
        /* found the correct screen with the handoff */
        if (true == screen->controllerManager().handoffRequired(callsign)) {
            auto controllers = screen->controllerManager().handoffStations(callsign);

            /* no handoff requested, but a release */
            if (true == release) {
                if (true == tracked)
                    radarTarget.GetCorrelatedFlightPlan().EndTracking();
                screen->controllerManager().handoffPerformed(callsign);
            }
            /* check if handoff or a release is needed */
            else if (1 == controllers.size()) {
                /* handoff to unicom */
                if (true == tracked) {
                    if (0 == controllers.front().size())
                        radarTarget.GetCorrelatedFlightPlan().EndTracking();
                    else
                        radarTarget.GetCorrelatedFlightPlan().InitiateHandoff(controllers.front().c_str());
                }
                screen->controllerManager().handoffPerformed(callsign);
            }
            else {
                auto& frequency = screen->controllerManager().handoffFrequency(callsign);
                this->OpenPopupList(area, "Handoff To", 2);

                for (const auto& controller : std::as_const(controllers)) {
                    this->AddPopupListElement(controller.c_str(), frequency.c_str(),
                        static_cast<int>(PlugIn::TagItemFunction::HandoffSectorChange));
                }
            }

            break;
        }
    }
}

void PlugIn::OnFunctionCall(int functionId, const char* itemString, POINT pt, RECT area) {
    (void)pt;

    auto radarTarget = this->RadarTargetSelectASEL();
    if (false == radarTarget.IsValid())
        return;
    std::string callsign(radarTarget.GetCallsign());

    /* check if we are tracking */
    bool tracked = radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe();

    switch (static_cast<PlugIn::TagItemFunction>(functionId)) {
    case PlugIn::TagItemFunction::HandoffInitiated:
        this->OpenPopupList(area, "Handoff initiate", 1);
        this->AddPopupListElement("Handoff", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform));
        if (true == tracked)
            this->AddPopupListElement("Release", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform));
        break;
    case PlugIn::TagItemFunction::HandoffPerform:
        if (0 == std::strncmp(itemString, "Release", 7))
            this->handleHandoffPerform(area, callsign, true, tracked);
        else
            this->handleHandoffPerform(area, callsign, false, tracked);
        break;
    case PlugIn::TagItemFunction::HandoffControllerSelect:
        for (const auto& screen : std::as_const(this->m_screens)) {
            /* found the correct screen with the handoff */
            if (true == screen->controllerManager().handoffRequired(callsign)) {
                if (true == tracked)
                    radarTarget.GetCorrelatedFlightPlan().InitiateHandoff(itemString);
                screen->controllerManager().handoffPerformed(callsign);
                break;
            }
        }
        break;
    case PlugIn::TagItemFunction::HandoffSectorChange:
        for (const auto& screen : std::as_const(this->m_screens)) {
            if (true == screen->controllerManager().handoffRequired(callsign)) {
                auto sectors = screen->controllerManager().handoffSectors();

                this->OpenPopupList(area, "Handoff sectors", 2);
                for (const auto& sector : std::as_const(sectors)) {
                    this->AddPopupListElement(sector.identifier().c_str(), sector.primaryFrequency().c_str(),
                                              static_cast<int>(PlugIn::TagItemFunction::HandoffSectorSelect));
                }

                break;
            }
        }
        break;
    case PlugIn::TagItemFunction::HandoffSectorSelect:
        for (const auto& screen : std::as_const(this->m_screens)) {
            if (true == screen->controllerManager().handoffRequired(callsign)) {
                screen->controllerManager().handoffSectorSelect(callsign, itemString);
                break;
            }
        }
        break;
    default:
        break;
    }
}
