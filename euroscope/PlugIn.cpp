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

    this->RegisterTagItemFunction("Menu bar", static_cast<int>(PlugIn::TagItemFunction::AircraftControlMenuBar));
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

bool PlugIn::visualizeManuallyAlerts(const types::Flight& flight, int idx, char itemString[16]) const {
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

    /* find the correct flight in the registries */
    types::Flight flight;
    for (const auto& screen : std::as_const(this->m_screens)) {
        if (true == screen->flightRegistry().flightExists(callsign)) {
            flight = screen->flightRegistry().flight(callsign);
            break;
        }
    }

    switch (static_cast<PlugIn::TagItemElement>(itemCode)) {
    case PlugIn::TagItemElement::HandoffFrequency:
        /* test all loaded screens */
        for (const auto& screen : std::as_const(this->m_screens)) {
            /* found the correct screen with the handoff */
            if (true == screen->sectorControl().handoffRequired(callsign)) {
                std::strcpy(itemString, screen->sectorControl().handoffFrequency(callsign).c_str());
                *colorCode = EuroScopePlugIn::TAG_COLOR_NOTIFIED;
                break;
            }
        }
        break;
    case PlugIn::TagItemElement::ManuallyAlerts0:
        if (true == this->visualizeManuallyAlerts(flight, 0, itemString))
            *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
        break;
    case PlugIn::TagItemElement::ManuallyAlerts1:
        if (true == this->visualizeManuallyAlerts(flight, 1, itemString))
            *colorCode = EuroScopePlugIn::TAG_COLOR_INFORMATION;
        break;
    case PlugIn::TagItemElement::ManuallyAlerts2:
        if (true == this->visualizeManuallyAlerts(flight, 2, itemString))
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
        if (true == screen->sectorControl().handoffRequired(callsign)) {
            auto controllers = screen->sectorControl().handoffStations(callsign);

            /* no handoff requested, but a release */
            if (true == release) {
                if (true == tracked)
                    radarTarget.GetCorrelatedFlightPlan().EndTracking();
                screen->sectorControl().handoffPerformed(callsign);
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
                screen->sectorControl().handoffPerformed(callsign);
            }
            else {
                auto& frequency = screen->sectorControl().handoffFrequency(callsign);
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
    case PlugIn::TagItemFunction::AircraftControlMenuBar:
        this->OpenPopupList(area, "Aircraft menu", 1);

        /* define the menu bar for the tracking functions */
        if (true == radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe()) {
            this->AddPopupListElement("Transfer", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform));
            this->AddPopupListElement("Release", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform));
        }
        else {
            this->AddPopupListElement("Assume", "", static_cast<int>(PlugIn::TagItemFunction::AircraftControlSelect));
            this->AddPopupListElement("Handoff", "", static_cast<int>(PlugIn::TagItemFunction::HandoffPerform));
        }
        this->AddPopupListElement("-------", "", 0);

        /* define the menu bar for the manually alerts */

        break;
    case PlugIn::TagItemFunction::AircraftControlSelect:
        if (0 == std::strncmp(itemString, "Assume", 6))
            radarTarget.GetCorrelatedFlightPlan().StartTracking();
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
            if (true == screen->sectorControl().handoffRequired(callsign)) {
                if (true == tracked)
                    radarTarget.GetCorrelatedFlightPlan().InitiateHandoff(itemString);
                screen->sectorControl().handoffPerformed(callsign);
                break;
            }
        }
        break;
    case PlugIn::TagItemFunction::HandoffSectorChange:
        for (const auto& screen : std::as_const(this->m_screens)) {
            if (true == screen->sectorControl().handoffRequired(callsign)) {
                auto sectors = screen->sectorControl().handoffSectors();

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
            if (true == screen->sectorControl().handoffRequired(callsign)) {
                screen->sectorControl().handoffSectorSelect(callsign, itemString);
                break;
            }
        }
        break;
    default:
        break;
    }
}
