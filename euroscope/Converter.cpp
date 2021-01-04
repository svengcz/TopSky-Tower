/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * License:
 *   LGPLv3
 * Brief:
 *   Implements the ES converter
 */

#include "stdafx.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

#include <helper/String.h>
#include <system/ConfigurationRegistry.h>

#include "Converter.h"

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
    case 'D':
    case 'Y':
        retval.setRnavCapable(true);
        break;
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
    retval.setDestination(plan.GetFlightPlanData().GetDestination());
    retval.setArrivalRoute(plan.GetFlightPlanData().GetStarName());
    retval.setClearanceLimit(static_cast<float>(plan.GetControllerAssignedData().GetClearedAltitude()) * types::feet);
    retval.setClearanceFlag(plan.GetClearenceFlag());

    if (nullptr != plan.GetControllerAssignedData().GetSquawk())
        retval.setAssignedSquawk(static_cast<std::uint16_t>(std::atoi(plan.GetControllerAssignedData().GetSquawk())));

    if (types::FlightPlan::Type::IFR == retval.type()) {
        auto& config = system::ConfigurationRegistry::instance().airportConfiguration(retval.origin());

        auto sidIt = config.sids.find(retval.departureRoute());
        if (config.sids.cend() != sidIt && sidIt->second.clearanceLimit != retval.clearanceLimit()) {
            retval.setClearanceLimit(sidIt->second.clearanceLimit);
            plan.GetControllerAssignedData().SetClearedAltitude(static_cast<int>(sidIt->second.clearanceLimit.convert(types::feet)));
        }
    }

    return retval;
}

types::Flight Converter::convert(const EuroScopePlugIn::CRadarTarget& target, const std::string& airport) {
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

        if (origin == destination) {
            /* TODO check if the AC is airborne or not */
        }
        else if (airport == origin) {
            retval.setType(types::Flight::Type::Departure);
        }
        else if (airport == destination) {
            retval.setType(types::Flight::Type::Arrival);
        }

        /* check if the flight is marked by a controller */
        std::string_view annotation(flightPlan.GetControllerAssignedData().GetFlightStripAnnotation(7));
        if (std::string::npos != annotation.find('K'))
            retval.setMarkedByController(true);

        /* analyze the scratch pad */
        std::string scratch = flightPlan.GetControllerAssignedData().GetScratchPadString();
        __analyzeScratchPad(scratch, retval);

        /* create the flight plan */
        retval.setFlightPlan(Converter::convert(flightPlan));
    }

    return retval;
}

types::ControllerInfo Converter::convert(const EuroScopePlugIn::CController& controller) {
    auto elements = helper::String::splitString(controller.GetCallsign(), "_");
    std::string prefix(elements[0]), midfix, suffix(elements.back());
    if (3 == elements.size())
        midfix = elements[1];

    /* transform callsign to upper cases */
#pragma warning(disable: 4244)
    std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::toupper);
    std::transform(midfix.begin(), midfix.end(), midfix.begin(), ::toupper);
    std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::toupper);
#pragma warning(default: 4244)

    /* transform the frequency into a string */
    std::stringstream stream;
    stream << std::fixed << std::setprecision(3) << controller.GetPrimaryFrequency();

    return types::ControllerInfo(controller.GetPositionId(), prefix, midfix, suffix,
                                 stream.str(), controller.GetFullName());
}
