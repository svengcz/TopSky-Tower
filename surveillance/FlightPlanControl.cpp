/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the flight plan check system
 */

#include <algorithm>

#include <surveillance/FlightPlanControl.h>
#include <system/ConfigurationRegistry.h>

using namespace topskytower;
using namespace topskytower::surveillance;
using namespace topskytower::types;

FlightPlanControl::FlightPlanControl() :
        m_flightChecks() { }

FlightPlanControl::~FlightPlanControl() {
    this->m_flightChecks.clear();
}

bool FlightPlanControl::validate(const types::Flight& flight) {
    /* ignore VFR flights */
    if (types::FlightPlan::Type::VFR == flight.flightPlan().type()) {
        this->m_flightChecks[flight.callsign()].errorCodes = { surveillance::FlightPlanControl::ErrorCode::VFR };
        return true;
    }

    /* validate the input flight plan */
    if (0 == flight.flightPlan().destination().length()) {
        this->removeFlight(flight.callsign());
        return false;
    }
    if (0 == flight.flightPlan().departureRoute().length()) {
        this->removeFlight(flight.callsign());
        return false;
    }
    if (types::FlightPlan::Type::Unknown == flight.flightPlan().type()) {
        this->removeFlight(flight.callsign());
        return false;
    }

    auto it = this->m_flightChecks.find(flight.callsign());

    /* check if a new validation is required */
    bool validationRequired = this->m_flightChecks.end() == it;
    if (false == validationRequired) {
        validationRequired |= flight.flightPlan().type() != it->second.type;
        validationRequired |= flight.flightPlan().departureRoute() != it->second.departureRoute;
        validationRequired |= flight.flightPlan().destination() != it->second.destination;
        validationRequired |= flight.flightPlan().flightLevel() != it->second.requestedFlightLevel;
        validationRequired |= flight.flightPlan().rnavCapable() != it->second.rnavCapable;
        validationRequired |= flight.flightPlan().transponderExists() != it->second.transponderAvailable;
    }
    else {
        this->m_flightChecks[flight.callsign()] = {
            {},
            false,
            "",
            "",
            types::FlightPlan::Type::Unknown,
            false,
            false,
            0_ft
        };

        it = this->m_flightChecks.find(flight.callsign());
    }

    /* validate the flight plan */
    if (true == validationRequired) {
        /* update the information */
        it->second.destination = flight.flightPlan().destination();
        it->second.departureRoute = flight.flightPlan().departureRoute();
        it->second.type =  flight.flightPlan().type();
        it->second.overwritten = false;
        it->second.rnavCapable = flight.flightPlan().rnavCapable();
        it->second.transponderAvailable = flight.flightPlan().transponderExists();
        it->second.requestedFlightLevel = flight.flightPlan().flightLevel();
        it->second.errorCodes.clear();

        const auto& config = system::ConfigurationRegistry::instance().airportConfiguration(flight.flightPlan().origin());

        /* no configuration for the SID found */
        auto sit = config.sids.find(it->second.departureRoute);
        if (config.sids.cend() == sit) {
            it->second.errorCodes.push_back(ErrorCode::DepartureRoute);
            return validationRequired;
        }

        /* the engine-type needs to be checked */
        if (types::Aircraft::EngineType::Unknown != sit->second.engineType) {
            if (sit->second.engineType != flight.flightPlan().aircraft().engineType())
                it->second.errorCodes.push_back(ErrorCode::EngineType);
        }

        /* RNAV is required */
        if (true == sit->second.requiresRnav && false == flight.flightPlan().rnavCapable())
            it->second.errorCodes.push_back(ErrorCode::Navigation);

        /* transponder is required */
        if (true == sit->second.requiresTransponder && false == flight.flightPlan().transponderExists())
            it->second.errorCodes.push_back(ErrorCode::Transponder);

        /* check the flight level constraints */
        if (sit->second.minimumCruiseLevel > flight.flightPlan().flightLevel() || sit->second.maximumCruiseLevel < flight.flightPlan().flightLevel())
            it->second.errorCodes.push_back(ErrorCode::FlightLevel);

        /* prepare for the even-odd checks */
        bool even = 0 == static_cast<int>(flight.flightPlan().flightLevel().convert(types::feet)) % 2000;

        /* check if a destination specific constraint is defined */
        bool foundDestinationConstraint = false;
        for (const auto& constraint : std::as_const(config.destinationConstraints)) {
            if (constraint.destination == flight.flightPlan().destination()) {
                if (true == constraint.evenCruiseLevel && false == even || false == constraint.evenCruiseLevel && true == even)
                    it->second.errorCodes.push_back(ErrorCode::EvenOddLevel);

                foundDestinationConstraint = true;
                break;
            }
        }

        if (false == foundDestinationConstraint &&
            true == system::ConfigurationRegistry::instance().systemConfiguration().flightPlanCheckEvenOdd)
        {
            if (0 == flight.flightPlan().route().waypoints().size()) {
                it->second.errorCodes.push_back(ErrorCode::Route);
            }
            else {
                /* find the bearing between start and destination */
                const auto& waypoints = flight.flightPlan().route().waypoints();
                auto bearing = waypoints[0].position().bearingTo(waypoints.back().position());

                /* use the half circle rule*/
                if (180_deg > bearing && true == even || 180_deg <= bearing && false == even)
                    it->second.errorCodes.push_back(ErrorCode::EvenOddLevel);
            }
        }

        /* no error found */
        if (0 == it->second.errorCodes.size())
            it->second.errorCodes.push_back(ErrorCode::NoError);
    }

    return validationRequired;
}

void FlightPlanControl::removeFlight(const std::string& callsign) {
    auto it = this->m_flightChecks.find(callsign);
    if (this->m_flightChecks.end() != it)
        this->m_flightChecks.erase(it);
}

void FlightPlanControl::overwrite(const std::string& callsign) {
    auto it = this->m_flightChecks.find(callsign);
    if (this->m_flightChecks.end() != it)
        it->second.overwritten = true;
}

const std::list<FlightPlanControl::ErrorCode>& FlightPlanControl::errorCodes(const std::string& callsign) const {
    static std::list<FlightPlanControl::ErrorCode> fallback{ FlightPlanControl::ErrorCode::Unknown };
    auto it = this->m_flightChecks.find(callsign);

    if (this->m_flightChecks.cend() != it)
        return it->second.errorCodes;
    else
        return fallback;
}

bool FlightPlanControl::overwritten(const std::string& callsign) const {
    auto it = this->m_flightChecks.find(callsign);

    if (this->m_flightChecks.cend() != it)
        return it->second.overwritten;
    else
        return false;
}

FlightPlanControl& FlightPlanControl::instance() {
    static FlightPlanControl instance;
    return instance;
}
