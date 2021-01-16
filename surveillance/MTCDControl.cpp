/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the Medium Term Conflict Detection system
 */

#include <surveillance/MTCDControl.h>

using namespace topskytower;
using namespace topskytower::surveillance;
using namespace topskytower::types;

MTCDControl::MTCDControl(const std::string& airport, const types::Coordinate& center) :
        management::HoldingPointMap<management::HoldingPointData>(airport, center),
        m_sidExtractionCallback(),
        m_departures(),
        m_conflicts() {
    system::ConfigurationRegistry::instance().registerNotificationCallback(this, &MTCDControl::reinitialize);

    this->reinitialize(system::ConfigurationRegistry::UpdateType::All);
}

void MTCDControl::reinitialize(system::ConfigurationRegistry::UpdateType type) {
    if (system::ConfigurationRegistry::UpdateType::All != type && system::ConfigurationRegistry::UpdateType::Airports != type)
        return;

    management::HoldingPointMap<management::HoldingPointData>::reinitialize();
}

static __inline types::Angle __normalize(const types::Angle& angle) {
    auto retval(angle);

    while (180.0_deg < retval)
        retval -= 360.0_deg;
    while (-1.0f * 180.0_deg > retval)
        retval += 360.0_deg;

    return retval;
}

std::list<DepartureModel>::iterator MTCDControl::insertFlight(const types::Flight& flight) {
    /* ignore non-departing flights and non-IFR flights */
    if (types::Flight::Type::Departure != flight.type() || types::FlightPlan::Type::IFR != flight.flightPlan().type())
        return this->m_departures.end();

    /* the flight is actually departing -> add it to the list */
    if (40_kn < flight.groundSpeed() || types::FlightPlan::AtcCommand::Departure == flight.flightPlan().departureFlag()) {
        auto route = this->m_sidExtractionCallback(flight.callsign());
        if (0 == route.size())
            return this->m_departures.end();

        this->m_departures.push_back(std::move(DepartureModel(flight, this->m_centerPosition, route)));
    }
    /* check if it is a departure candidate */
    else {
        /* get more than one to handle also the active runways */
        auto nodes = this->findNextHoldingPoints<1>(flight);
        bool inserted = false;

        /* check if the holding point is correct */
        if (nullptr != nodes[0] && flight.flightPlan().departureRunway() == nodes[0]->runway) {
            /* check if the heading is comparable */
            if (20_deg >= __normalize(nodes[0]->heading - flight.currentPosition().heading())) {
                /* check if we are close enough to the holding point */
                auto distance = nodes[0]->holdingPoint.distanceTo(flight.currentPosition().coordinate());
                if (distance <= system::ConfigurationRegistry::instance().systemConfiguration().ariwsMaximumDistance) {
                    this->m_departures.push_back(std::move(DepartureModel(flight, this->m_centerPosition,
                                                 this->m_sidExtractionCallback(flight.callsign()))));
                    inserted = true;
                }
            }
        }

        if (false == inserted)
            return this->m_departures.end();
    }

    /* get the last inserted element */
    auto it = this->m_departures.begin();
    std::advance(it, this->m_departures.size() - 1);
    return it;
}

void MTCDControl::updateFlight(const types::Flight& flight) {
    /* the controller disabled the system */
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().mtcdActive ||
        false == system::ConfigurationRegistry::instance().runtimeConfiguration().mtcdActive)
    {
        return;
    }

    /* the callback function is not registered -> no prediction possible */
    if (nullptr == this->m_sidExtractionCallback)
        return;

    auto it = std::find(this->m_departures.begin(), this->m_departures.end(), DepartureModel(flight.callsign()));

    /* we've got a new candidate -> check how to insert it */
    if (this->m_departures.end() == it) {
        it = this->insertFlight(flight);
        /* did not insert it -> stop any processing */
        if (this->m_departures.end() == it)
            return;
    }
    /* update the internal states */
    else {
        it->update(flight, this->m_sidExtractionCallback(flight.callsign()));
    }

    /* check if the flight reached the SIDs exit */
    if (0 == it->waypoints().size()) {
        this->removeFlight(flight.callsign());
        return;
    }

    /* do not check departed flights */
    if (types::FlightPlan::AtcCommand::Departure == flight.flightPlan().departureFlag() || 40_kn < flight.groundSpeed()) {
        /* erase flights where this flight is the initiator of the conflict */
        for (auto cit = this->m_conflicts.begin(); this->m_conflicts.end() != cit;) {
            if (cit->callsigns[0] == flight.callsign())
                cit = this->m_conflicts.erase(cit);
            else
                ++cit;
        }

        return;
    }

    /* find intersections between all candidates */
    const auto& config = system::ConfigurationRegistry::instance().systemConfiguration();
    for (const auto& departure : std::as_const(this->m_departures)) {
        if (*it != departure) {
            auto candidates = it->findConflictCandidates(departure);
            auto minVerticalSpacing = it->flight().flightPlan().destination() != departure.flight().flightPlan().destination() ?
                config.mtcdVerticalSeparation : config.mtcdVerticalSeparationSameDestination;

            /* check all candidates */
            for (const auto& candidate : std::as_const(candidates)) {
                /* found a critical conflict */
                if (candidate.altitudeDifference < minVerticalSpacing && candidate.horizontalSpacing < config.mtcdHorizontalSeparation) {
                    Conflict conflict = {
                        { it->flight().callsign(), departure.flight().callsign() },
                        candidate
                    };

                    this->m_conflicts.push_back(std::move(conflict));
                    return;
                }
            }
        }
    }
}

void MTCDControl::removeFlight(const std::string& callsign) {
    auto it = std::find(this->m_departures.begin(), this->m_departures.end(), DepartureModel(callsign));
    if (this->m_departures.end() != it) {
        this->m_departures.erase(it);
        return;
    }

    for (auto cit = this->m_conflicts.begin(); this->m_conflicts.end() != cit;) {
        if (cit->callsigns[0] == callsign || cit->callsigns[1] == callsign)
            cit = this->m_conflicts.erase(cit);
        else
            ++cit;
    }
}

std::size_t MTCDControl::numberOfConflicts(const types::Flight& flight) const {
    std::size_t retval = 0;

    /* the controller disabled the system */
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().mtcdActive ||
        false == system::ConfigurationRegistry::instance().runtimeConfiguration().mtcdActive)
    {
        return 0;
    }

    for (const auto& conflict : std::as_const(this->m_conflicts)) {
        if (flight.callsign() == conflict.callsigns[0])
            retval += 1;
    }

    return retval;
}

const MTCDControl::Conflict& MTCDControl::conflict(const types::Flight& flight, std::size_t idx) const {
    static Conflict __fallback;

    /* the controller disabled the system */
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().mtcdActive ||
        false == system::ConfigurationRegistry::instance().runtimeConfiguration().mtcdActive)
    {
        return __fallback;
    }

    for (const auto& conflict : std::as_const(this->m_conflicts)) {
        if (flight.callsign() == conflict.callsigns[0] && 0 == idx--)
            return conflict;
    }

    return __fallback;
}
