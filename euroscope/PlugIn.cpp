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

#include <version.h>

#include "PlugIn.h"

using namespace topskytower::euroscope;

PlugIn::PlugIn() :
        EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
                                 PLUGIN_NAME,
                                 PLUGIN_VERSION_BUILD,
                                 PLUGIN_DEVELOPER,
                                 PLUGIN_COPYRIGHT) { }

PlugIn::~PlugIn() { }

EuroScopePlugIn::CRadarScreen* PlugIn::OnRadarScreenCreated(const char* displayName, bool needsRadarContent, bool geoReferenced,
                                                            bool canBeSaved, bool canBeCreated) {
    (void)needsRadarContent;
    (void)geoReferenced;
    (void)canBeSaved;
    (void)canBeCreated;
    (void)displayName;
    return nullptr;
}

void PlugIn::OnAirportRunwayActivityChanged() { }

bool PlugIn::OnCompileCommand(const char* cmdline) {
    (void)cmdline;
    return false;
}

void PlugIn::OnGetTagItem(EuroScopePlugIn::CFlightPlan flightPlan, EuroScopePlugIn::CRadarTarget radarTarget,
                          int itemCode, int tagData, char itemString[16], int* colorCode, COLORREF* rgb,
                          double* fontSize) {
    (void)flightPlan;
    (void)radarTarget;
    (void)itemCode;
    (void)itemString;
    (void)colorCode;
    (void)fontSize;
    (void)tagData;
    (void)rgb;
}

void PlugIn::OnFunctionCall(int functionId, const char* itemString, POINT pt, RECT area) {
    (void)functionId;
    (void)itemString;
    (void)pt;
    (void)area;
}
