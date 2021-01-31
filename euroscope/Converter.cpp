/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the ES converter
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

#include <helper/String.h>
#include <system/ConfigurationRegistry.h>

#include "Converter.h"
#include "PlugIn.h"
#include "RadarScreen.h"

using namespace topskytower;
using namespace topskytower::euroscope;

types::Coordinate Converter::convert(const EuroScopePlugIn::CPosition& position) {
    return types::Coordinate(static_cast<float>(position.m_Longitude) * types::degree,
                             static_cast<float>(position.m_Latitude) * types::degree);
}

static void __analyzeScratchPad(const std::string& scratchPad, types::Flight& flight) {
    /* check if the entries are set */
    if (std::string::npos == scratchPad.find('_'))
        return;

    /* check the different entries */
    if (std::string::npos != scratchPad.find("MISAP_"))
        flight.setOnMissedApproach(true);
    if (std::string::npos != scratchPad.find("IRREG_"))
        flight.setIrregularHandoff(true);
    if (std::string::npos != scratchPad.find("EST_"))
        flight.setEstablishedOnILS(true);
}

static __inline types::Aircraft __translate(const std::string& code, char wtc) {
    const auto& aircrafts = system::ConfigurationRegistry::instance().aircrafts();
    types::Aircraft retval;

    auto it = aircrafts.find(code);
    if (aircrafts.cend() != it) {
        retval = it->second;
    }
    else {
        switch (wtc) {
        case 'L':
            retval = aircrafts.find("C172")->second;
            break;
        case 'H':
            retval = aircrafts.find("B744")->second;
            break;
        case 'J':
            retval = aircrafts.find("A388")->second;
            break;
        case 'M':
        default:
            retval = aircrafts.find("A320")->second;
            break;
        }
    }

    switch (wtc) {
    case 'L':
        retval.setWTC(types::Aircraft::WTC::Light);
        break;
    case 'H':
        retval.setWTC(types::Aircraft::WTC::Heavy);
        break;
    case 'J':
        retval.setWTC(types::Aircraft::WTC::Super);
        break;
    case 'M':
    default:
        retval.setWTC(types::Aircraft::WTC::Medium);
        break;
    }

    return retval;
}

static __inline void __convertEuroScopeAtcStatus(const std::string_view& sts, types::FlightPlan& plan) {
    if ("ST-UP" == sts)
        plan.setFlag(types::FlightPlan::AtcCommand::StartUp);
    else if ("PUSH" == sts)
        plan.setFlag(types::FlightPlan::AtcCommand::Pushback);
    else if ("TAXI" == sts)
        plan.setFlag(types::FlightPlan::AtcCommand::TaxiOut);
    else if ("DEPA" == sts)
        plan.setFlag(types::FlightPlan::AtcCommand::Departure);
}

static __inline void __convertTopSkyTowerAtcPad(const std::string& entry, types::FlightPlan& flightPlan) {
    int mask = std::atoi(entry.c_str());
    std::uint16_t departure = static_cast<std::uint16_t>(mask & 0x0ff);
    std::uint16_t arrival = static_cast<std::uint16_t>(mask & 0xf00);

    /* validate the values to avoid buffer overruns */
    if (departure <= static_cast<std::uint16_t>(types::FlightPlan::AtcCommand::Departure) &&
        arrival <= static_cast<std::uint16_t>(types::FlightPlan::AtcCommand::TaxiIn))
    {
        flightPlan.setFlag(static_cast<types::FlightPlan::AtcCommand>(departure));
        flightPlan.setFlag(static_cast<types::FlightPlan::AtcCommand>(arrival));
    }
}

std::string Converter::findScratchPadEntry(const EuroScopePlugIn::CFlightPlan& plan, const std::string& marker, const std::string& entry) {
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

void Converter::convertAtcCommand(const EuroScopePlugIn::CFlightPlan& plan, types::FlightPlan& flightPlan) {
    /* get the ground status and check if this or an other TopSky-Tower set something */
    __convertEuroScopeAtcStatus(plan.GetGroundState(), flightPlan);

    std::string scratchpad = plan.GetControllerAssignedData().GetScratchPadString();
    bool updated = true;
    std::size_t pos;

    if (std::string::npos != (pos = scratchpad.find("DE-ICE"))) {
        flightPlan.setFlag(types::FlightPlan::AtcCommand::Deicing);
        scratchpad.erase(pos, 6);
    }
    else if (std::string::npos != (pos = scratchpad.find("LINEUP"))) {
        flightPlan.setFlag(types::FlightPlan::AtcCommand::LineUp);
        scratchpad.erase(pos, 6);
    }
    else if (std::string::npos != (pos = scratchpad.find("APPROACH"))) {
        flightPlan.setFlag(types::FlightPlan::AtcCommand::Approach);
        scratchpad.erase(pos, 8);
    }
    else if (std::string::npos != (pos = scratchpad.find("LANDING"))) {
        flightPlan.setFlag(types::FlightPlan::AtcCommand::Land);
        scratchpad.erase(pos, 7);
    }
    else if (std::string::npos != (pos = scratchpad.find("TAXIIN"))) {
        flightPlan.setFlag(types::FlightPlan::AtcCommand::TaxiIn);
        scratchpad.erase(pos, 6);
    }
    else if (std::string::npos != (pos = scratchpad.find("GOAROUND"))) {
        flightPlan.setFlag(types::FlightPlan::AtcCommand::GoAround);
        scratchpad.erase(pos, 8);
    }
    else {
        updated = false;
    }

    /* update the internal entries and the scratch pad */
    if (true == updated)
        plan.GetControllerAssignedData().SetScratchPadString(scratchpad.c_str());
}

types::FlightPlan Converter::convert(const EuroScopePlugIn::CFlightPlan& plan) {
    types::FlightPlan retval;

    switch (plan.GetFlightPlanData().GetPlanType()[0]) {
    case 'V':
        retval.setType(types::FlightPlan::Type::VFR);
        break;
    case 'I':
        retval.setType(types::FlightPlan::Type::IFR);
        break;
    default:
        break;
    }

    /* translate the capabilities-entry */
    switch (plan.GetFlightPlanData().GetCapibilities()) {
    case 'T':
    case 'U':
        retval.setTransponderExistence(true);
        break;
    case 'Y':
    case 'M':
    case 'B':
    case 'A':
    case 'N':
    case 'H':
    case 'S':
    case 'P':
    case 'C':
    case 'I':
    case 'E':
    case 'F':
    case 'G':
    case 'R':
    case 'W':
    case 'Q':
    case 'L':
    case 'D':
    case 'K':
        retval.setRnavCapable(true);
        retval.setTransponderExistence(true);
        break;
    case 'X':
    default:
        break;
    }

    /* translate the aircraft and set some specific values */
    auto aircraft = __translate(plan.GetFlightPlanData().GetAircraftFPType(), plan.GetFlightPlanData().GetAircraftWtc());
    aircraft.setEngineCount(static_cast<std::uint8_t>(plan.GetFlightPlanData().GetEngineNumber()));
    switch (plan.GetFlightPlanData().GetEngineType()) {
    case 'P':
    case 'T':
        aircraft.setEngineType(types::Aircraft::EngineType::Turboprop);
        break;
    case 'E':
        aircraft.setEngineType(types::Aircraft::EngineType::Electric);
        break;
    case 'J':
    default:
        aircraft.setEngineType(types::Aircraft::EngineType::Jet);
        break;
    }

    /* define the route information */
    retval.setAircraft(aircraft);
    retval.setOrigin(plan.GetFlightPlanData().GetOrigin());
    retval.setDepartureRoute(plan.GetFlightPlanData().GetSidName());
    retval.setDepartureRunway(plan.GetFlightPlanData().GetDepartureRwy());
    retval.setDestination(plan.GetFlightPlanData().GetDestination());
    retval.setArrivalRunway(plan.GetFlightPlanData().GetArrivalRwy());
    retval.setFlightLevel(static_cast<float>(plan.GetFlightPlanData().GetFinalAltitude()) * types::feet);
    retval.setArrivalRoute(plan.GetFlightPlanData().GetStarName());
    retval.setClearanceLimit(static_cast<float>(plan.GetControllerAssignedData().GetClearedAltitude()) * types::feet);
    retval.setClearanceFlag(plan.GetClearenceFlag());

    Converter::convertAtcCommand(plan, retval);

    /* convert the route */
    std::vector<types::Waypoint> waypoints;
    waypoints.reserve(plan.GetExtractedRoute().GetPointsNumber());
    for (int i = 0; i < plan.GetExtractedRoute().GetPointsNumber(); ++i) {
        waypoints.push_back(std::move(types::Waypoint(plan.GetExtractedRoute().GetPointName(i),
                                                      Converter::convert(plan.GetExtractedRoute().GetPointPosition(i)))));
    }
    types::Route route(std::move(waypoints));
    retval.setRoute(std::move(route));

    if (nullptr != plan.GetControllerAssignedData().GetSquawk())
        retval.setAssignedSquawk(static_cast<std::uint16_t>(std::atoi(plan.GetControllerAssignedData().GetSquawk())));

    return retval;
}

void Converter::convertStandAssignment(const EuroScopePlugIn::CFlightPlan& plan, const types::Flight& flight, RadarScreen& screen) {
    auto stand = Converter::findScratchPadEntry(plan, "GRP", "S");
    if (0 != stand.length()) {
        screen.standControl().assignManually(flight, stand);
        plan.GetControllerAssignedData().SetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Stand), ("s/" + stand + "/s").c_str());
    }
}

types::Flight Converter::convert(const EuroScopePlugIn::CRadarTarget& target, RadarScreen& screen) {
    types::Flight retval(target.GetCallsign());

    retval.setGroundSpeed(static_cast<float>(target.GetPosition().GetReportedGS()) * types::knot);
    retval.setVerticalSpeed(static_cast<float>(target.GetVerticalSpeed()) * (types::feet / types::minute));
    types::Position position(Converter::convert(target.GetPosition().GetPosition()),
                             static_cast<float>(target.GetPosition().GetPressureAltitude()) * types::feet,
                             static_cast<float>(target.GetPosition().GetReportedHeading()) * types::degree);
    retval.setCurrentPosition(position);

    auto flightPlan = target.GetCorrelatedFlightPlan();
    if (true == flightPlan.IsValid()) {
        std::string_view origin(flightPlan.GetFlightPlanData().GetOrigin());
        std::string_view destination(flightPlan.GetFlightPlanData().GetDestination());

        retval.setTrackedState(flightPlan.GetTrackingControllerIsMe());
        bool isTrackedByOther = nullptr != flightPlan.GetTrackingControllerId() && 0 != std::strlen(flightPlan.GetTrackingControllerId());
        isTrackedByOther &= false == retval.isTracked();
        retval.setTrackedByOtherState(isTrackedByOther);
        /* an flight was tracked by an other controller and we keep this information */
        if (EuroScopePlugIn::FLIGHT_PLAN_STATE_TRANSFER_TO_ME_INITIATED == flightPlan.GetState()) {
            if (nullptr != flightPlan.GetTrackingControllerId() && 0 != std::strlen(flightPlan.GetTrackingControllerId()))
                retval.setHandoffInitiatedId(flightPlan.GetTrackingControllerId());
        }
        /* get the old handoff information */
        else if (true == screen.flightRegistry().flightExists(retval.callsign()) && true == retval.isTracked()) {
            retval.setHandoffInitiatedId(screen.flightRegistry().flight(retval.callsign()).handoffInitiatedId());
        }

        /* check if the flight is marked by a controller */
        std::string_view annotation(flightPlan.GetControllerAssignedData().GetFlightStripAnnotation(static_cast<int>(PlugIn::AnnotationIndex::Marker)));
        if (std::string::npos != annotation.find('K'))
            retval.setMarkedByController(true);

        /* analyze the scratch pad */
        std::string scratch = flightPlan.GetControllerAssignedData().GetScratchPadString();
        __analyzeScratchPad(scratch, retval);

        /* create the flight plan */
        retval.setFlightPlan(Converter::convert(flightPlan));

        if (origin == destination) {
            if (retval.flightPlan().departureFlag() == types::FlightPlan::AtcCommand::Departure)
                retval.setType(types::Flight::Type::Arrival);
            else
                retval.setType(types::Flight::Type::Departure);
        }
        else if (screen.airportIcao() == origin) {
            retval.setType(types::Flight::Type::Departure);
        }
        else if (screen.airportIcao() == destination) {
            retval.setType(types::Flight::Type::Arrival);
        }
    }

    Converter::convertStandAssignment(target.GetCorrelatedFlightPlan(), retval, screen);

    return retval;
}

types::ControllerInfo Converter::convert(const EuroScopePlugIn::CController& controller) {
    std::string callsign(controller.GetCallsign());

    /* transform callsign to upper cases */
#pragma warning(disable: 4244)
    std::transform(callsign.begin(), callsign.end(), callsign.begin(), ::toupper);
#pragma warning(default: 4244)

    /* transform the frequency into a string */
    std::stringstream stream;
    stream << std::fixed << std::setprecision(3) << controller.GetPrimaryFrequency();

    return types::ControllerInfo(controller.GetPositionId(), callsign, stream.str(), controller.GetFullName());
}
