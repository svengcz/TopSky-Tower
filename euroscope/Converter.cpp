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
#include <surveillance/ConfigurationRegistry.h>

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

    retval.setOrigin(plan.GetFlightPlanData().GetOrigin());
    retval.setDepartureRoute(plan.GetFlightPlanData().GetSidName());
    retval.setDestination(plan.GetFlightPlanData().GetDestination());
    retval.setArrivalRoute(plan.GetFlightPlanData().GetStarName());
    retval.setClearanceLimit(static_cast<float>(plan.GetControllerAssignedData().GetClearedAltitude()) * types::feet);
    retval.setClearedFlag(plan.GetClearenceFlag());

    if (nullptr != plan.GetControllerAssignedData().GetSquawk())
        retval.setAssignedSquawk(static_cast<std::uint16_t>(std::atoi(plan.GetControllerAssignedData().GetSquawk())));

    if (types::FlightPlan::Type::IFR == retval.type()) {
        auto& config = surveillance::ConfigurationRegistry::instance().airportConfiguration(retval.origin());

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
