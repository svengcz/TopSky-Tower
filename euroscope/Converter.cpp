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
#include <iomanip>
#include <sstream>

#include <helper/String.h>

#include "Converter.h"

using namespace topskytower;
using namespace topskytower::euroscope;

types::Coordinate Converter::convert(const EuroScopePlugIn::CPosition& position) {
    return types::Coordinate(static_cast<float>(position.m_Longitude) * types::degree,
                             static_cast<float>(position.m_Latitude) * types::degree);
}

types::Flight Converter::convert(const EuroScopePlugIn::CRadarTarget& target, const std::string& airport) {
    types::Flight retval(target.GetCallsign());

    retval.setGroundSpeed(static_cast<float>(target.GetGS()) * types::knot);
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
    }

    return retval;
}

types::ControllerInfo Converter::convert(const EuroScopePlugIn::CController& controller) {
    auto elements = helper::String::splitString(controller.GetCallsign(), "_");
    std::string prefix(elements[0]), midfix, suffix(elements.back());
    if (3 == elements.size())
        midfix = elements[1];

    /* transform callsign to upper cases */
    std::transform(prefix.begin(), prefix.end(), prefix.begin(), std::toupper);
    std::transform(midfix.begin(), midfix.end(), midfix.begin(), std::toupper);
    std::transform(suffix.begin(), suffix.end(), suffix.begin(), std::toupper);

    /* transform the frequency into a string */
    std::stringstream stream;
    stream << std::fixed << std::setprecision(3) << controller.GetPrimaryFrequency();

    return types::ControllerInfo(controller.GetPositionId(), prefix, midfix, suffix, stream.str());
}
