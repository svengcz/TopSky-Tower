/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the Medium Term Conflict Detection system
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <surveillance/MTCDControl.h>

using namespace topskytower;
using namespace topskytower::surveillance;
using namespace topskytower::types;

MTCDControl::MTCDControl(const types::Coordinate& center, management::DepartureSequenceControl* departureControl) :
        m_center(center),
        m_departureControl(departureControl),
        m_sidExtractionCallback(),
        m_departures(),
        m_conflicts() { }

std::list<DepartureModel>::iterator MTCDControl::insertFlight(const types::Flight& flight, types::Flight::Type type) {
    /* ignore non-departing flights and non-IFR flights */
    if (types::Flight::Type::Departure != type || types::FlightPlan::Type::IFR != flight.flightPlan().type())
        return this->m_departures.end();

    /* the flight is actually departing -> add it to the list */
    if (40_kn < flight.groundSpeed() || types::FlightPlan::AtcCommand::Departure == flight.flightPlan().departureFlag()) {
        auto route = this->m_sidExtractionCallback(flight.callsign());
        if (0 == route.size())
            return this->m_departures.end();

        this->m_departures.push_back(DepartureModel(flight, this->m_center, route));
    }
    /* check if it is a departure candidate */
    else if (true == this->m_departureControl->readyForDeparture(flight)) {
        this->m_departures.push_back(DepartureModel(flight, this->m_center, this->m_sidExtractionCallback(flight.callsign())));
    }
    else {
        return this->m_departures.end();
    }

    /* get the last inserted element */
    auto it = this->m_departures.begin();
    std::advance(it, this->m_departures.size() - 1);
    return it;
}

void MTCDControl::updateFlight(const types::Flight& flight, types::Flight::Type type) {
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
        it = this->insertFlight(flight, type);
        /* did not insert it -> stop any processing */
        if (this->m_departures.end() == it)
            return;
    }
    /* update the internal states */
    else {
        if (false == this->m_departureControl->readyForDeparture(flight)) {
            this->m_departures.erase(it);
            return;
        }
        else {
            it->update(flight, this->m_sidExtractionCallback(flight.callsign()));
        }
    }

    /* check if the flight reached the SIDs exit */
    if (0 == it->waypoints().size()) {
        this->removeFlight(flight.callsign());
        return;
    }

    /* do not check departed flights */
    if (types::FlightPlan::AtcCommand::Departure == flight.flightPlan().departureFlag() || 40_kn < flight.groundSpeed()) {
        /* erase flights where this flight is the initiator of the conflict */
        auto cit = this->m_conflicts.find(flight.callsign());
        if (this->m_conflicts.end() != cit)
            this->m_conflicts.erase(cit);
        return;
    }

    /* find intersections between all candidates */
    const auto& config = system::ConfigurationRegistry::instance().systemConfiguration();
    for (const auto& departure : std::as_const(this->m_departures)) {
        if (*it != departure) {
            auto candidates = it->findConflictCandidates(departure);

            /* erase all existing conflicts for this combination */
            if (0 == candidates.size()) {
                this->removeConflict(it->flight().callsign(), departure.flight().callsign());
                continue;
            }

            auto minVerticalSpacing = it->flight().flightPlan().destination() != departure.flight().flightPlan().destination() ?
                config.mtcdVerticalSeparation : config.mtcdVerticalSeparationSameDestination;

            candidates.sort([](const DepartureModel::ConflictPosition& c0, const DepartureModel::ConflictPosition& c1) {
                return c0.conflictIn < c1.conflictIn;
            });

            /* check all candidates */
            for (const auto& candidate : std::as_const(candidates)) {
                /* found a critical conflict */
                if (candidate.altitudeDifference < minVerticalSpacing && candidate.horizontalSpacing < config.mtcdHorizontalSeparation) {
                    Conflict conflict = {
                        departure.flight().callsign(),
                        candidate
                    };

                    /* update the statistic */
                    bool updated = false;
                    auto& conflicts = this->m_conflicts[it->flight().callsign()];
                    for (auto cit = conflicts.begin(); conflicts.end() != cit; ++cit) {
                        if (cit->callsign == departure.flight().callsign()) {
                            *cit = conflict;
                            updated = true;
                            break;
                        }
                    }

                    if (false == updated)
                        conflicts.push_back(std::move(conflict));
                    return;
                }
            }

            /* no relevant conflict found -> delete the old conflicts */
            this->removeConflict(it->flight().callsign(), departure.flight().callsign());
        }
    }
}

void MTCDControl::removeConflict(const std::string& callsignModel, const std::string& callsignConflict) {
    auto it = this->m_conflicts.find(callsignModel);
    if (this->m_conflicts.end() != it) {
        for (auto cit = it->second.begin(); it->second.end() != cit; ++cit) {
            if (cit->callsign == callsignConflict) {
                it->second.erase(cit);
                return;
            }
        }
    }
}

void MTCDControl::removeFlight(const std::string& callsign) {
    auto it = std::find(this->m_departures.begin(), this->m_departures.end(), DepartureModel(callsign));
    if (this->m_departures.end() != it)
        this->m_departures.erase(it);

    for (auto cit = this->m_conflicts.begin(); this->m_conflicts.end() != cit; ++cit) {
        for (auto oit = cit->second.begin(); oit != cit->second.end();) {
            if (oit->callsign == callsign)
                oit = cit->second.erase(oit);
            else
                ++oit;
        }
    }

    auto cit = this->m_conflicts.find(callsign);
    if (this->m_conflicts.end() != cit)
        this->m_conflicts.erase(cit);
}

bool MTCDControl::departureModelExists(const types::Flight& flight) const {
    auto it = std::find(this->m_departures.cbegin(), this->m_departures.cend(), DepartureModel(flight.callsign()));
    return this->m_departures.cend() != it;
}

const DepartureModel& MTCDControl::departureModel(const types::Flight& flight) const {
    static DepartureModel __fallback("");

    auto it = std::find(this->m_departures.cbegin(), this->m_departures.cend(), DepartureModel(flight.callsign()));
    if (this->m_departures.cend() != it)
        return *it;
    else
        return __fallback;
}

bool MTCDControl::conflictsExist(const types::Flight& flight) const {
    /* the controller disabled the system */
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().mtcdActive ||
        false == system::ConfigurationRegistry::instance().runtimeConfiguration().mtcdActive)
    {
        return false;
    }

    auto it = this->m_conflicts.find(flight.callsign());
    if (this->m_conflicts.cend() != it)
        return 0 != it->second.size();

    return false;
}

const std::list<MTCDControl::Conflict>& MTCDControl::conflicts(const types::Flight& flight) const {
    static std::list<Conflict> __fallback;

    /* the controller disabled the system */
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().mtcdActive ||
        false == system::ConfigurationRegistry::instance().runtimeConfiguration().mtcdActive)
    {
        return __fallback;
    }

    /* find the conflict */
    auto it = this->m_conflicts.find(flight.callsign());
    if (this->m_conflicts.cend() != it)
        return it->second;

    return __fallback;
}
