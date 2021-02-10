/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the departure sequence control system
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <algorithm>

#include <management/DepartureSequenceControl.h>
#include <system/FlightRegistry.h>

#include <system/Separation.h>

using namespace topskytower;
using namespace topskytower::management;
using namespace topskytower::types;

DepartureSequenceControl::DepartureSequenceControl(const std::string& airport, const types::Coordinate& center) :
        m_airport(airport),
        m_holdingPoints(airport, center),
        m_departureReady(),
        m_departedPerRunway() {
    system::ConfigurationRegistry::instance().registerNotificationCallback(this, &DepartureSequenceControl::reinitialize);
    this->reinitialize(system::ConfigurationRegistry::UpdateType::All);
}

void DepartureSequenceControl::reinitialize(system::ConfigurationRegistry::UpdateType type) {
    if (system::ConfigurationRegistry::UpdateType::All != type && system::ConfigurationRegistry::UpdateType::Runtime != type)
        return;

    this->m_holdingPoints.reinitialize();

    const auto& runways = system::ConfigurationRegistry::instance().runtimeConfiguration().activeDepartureRunways.find(this->m_airport)->second;
    bool runwayDisabled = false;

    /* delete deactivated runways */
    for (auto it = this->m_departedPerRunway.begin(); this->m_departedPerRunway.end() != it;) {
        auto rIt = std::find(runways.cbegin(), runways.cend(), it->first);
        if (runways.cend() == rIt) {
            it = this->m_departedPerRunway.erase(it);
            runwayDisabled = true;
        }
        else {
            ++it;
        }
    }

    /* insert activated runways */
    for (const auto& depRunway : std::as_const(runways)) {
        if (this->m_departedPerRunway.cend() == this->m_departedPerRunway.find(depRunway))
            this->m_departedPerRunway[depRunway] = DepartureInformation();
    }

    /* cleanup the old departures, if a runway is deactivated */
    if (true == runwayDisabled) {
        for (auto it = this->m_departureReady.begin(); this->m_departureReady.end() != it;) {
            if (false == system::FlightRegistry::instance().flightExists(it->first)) {
                it = this->m_departureReady.erase(it);
            }
            else {
                const auto& flight = system::FlightRegistry::instance().flight(it->first);
                auto rIt = this->m_departedPerRunway.find(flight.flightPlan().departureRunway());
                if (this->m_departedPerRunway.cend() == rIt)
                    it = this->m_departureReady.erase(it);
                else
                    ++it;
            }
        }
    }
}

void DepartureSequenceControl::updateFlight(const types::Flight& flight, types::Flight::Type type) {
    /* ignore non-departure or non-IFR flights */
    if (types::Flight::Type::Departure != type || types::FlightPlan::Type::IFR != flight.flightPlan().type())
        return;

    /* get the flags to check the holding points */
    std::size_t holdingPointIdx;
    bool atHoldingPoint = false, passedHoldingPoint = false;
    auto deadband = system::ConfigurationRegistry::instance().systemConfiguration().ariwsDistanceDeadband;
    if (true == this->m_holdingPoints.reachedHoldingPoint(flight, type, true, deadband, 20_deg, &holdingPointIdx))
        atHoldingPoint = true;
    else if (true == this->m_holdingPoints.passedHoldingPoint(flight, type, true, deadband, 20_deg, nullptr))
        passedHoldingPoint = true;

    auto rdyIt = this->m_departureReady.find(flight.callsign());

    /* update the internal statistics and check if the flight is departing */
    if (this->m_departureReady.cend() != rdyIt) {
        rdyIt->second.reachedHoldingPoint = atHoldingPoint;
        if (true == passedHoldingPoint)
            rdyIt->second.passedHoldingPoint = true;

        /* flight is departing -> update the internal statistics */
        if ((types::FlightPlan::AtcCommand::Departure == flight.flightPlan().departureFlag() && true == rdyIt->second.passedHoldingPoint) || 40_kn <= flight.groundSpeed()) {
            auto rwyIt = this->m_departedPerRunway.find(flight.flightPlan().departureRunway());
            if (this->m_departedPerRunway.end() != rwyIt) {
                rwyIt->second = std::move(rdyIt->second);
                rwyIt->second.actualTakeOffTime = std::chrono::system_clock::now();
                rwyIt->second.lastReportedPosition = flight.currentPosition().coordinate();
                rwyIt->second.flewDistance = 0.0_m;
            }

            /* cleanup the ready list */
            this->m_departureReady.erase(rdyIt);
        }
        /* flight is not ready anymore and not at the holding point */
        else if (false == flight.readyForDeparture() && false == atHoldingPoint && false == rdyIt->second.passedHoldingPoint) {
            this->m_departureReady.erase(rdyIt);
        }
        else if (true == atHoldingPoint) {
            rdyIt->second.normalProcedureHoldingPoint = false == system::ConfigurationRegistry::instance().runtimeConfiguration().lowVisibilityProcedures;
            rdyIt->second.holdingPoint = this->m_holdingPoints.holdingPoint(rdyIt->second.normalProcedureHoldingPoint, holdingPointIdx);
        }
    }
    /* check if it is a candidate to be departure ready */
    else if (40_kn > flight.groundSpeed() && (true == flight.readyForDeparture() || true == atHoldingPoint)) {
        bool normalProcedure = false == system::ConfigurationRegistry::instance().runtimeConfiguration().lowVisibilityProcedures;

        DepartureInformation info = {
            flight.callsign(),
            atHoldingPoint,
            passedHoldingPoint,
            normalProcedure,
            this->m_holdingPoints.holdingPoint(normalProcedure, holdingPointIdx),
            flight.flightPlan().aircraft().wtc(),
            TimePoint()
        };

        this->m_departureReady[flight.callsign()] = std::move(info);
    }
    /* check if the flight departed -> update the internal tracking */
    else {
        auto rwyIt = this->m_departedPerRunway.find(flight.flightPlan().departureRunway());
        if (this->m_departedPerRunway.end() != rwyIt && flight.callsign() == rwyIt->second.callsign) {
            rwyIt->second.flewDistance += rwyIt->second.lastReportedPosition.distanceTo(flight.currentPosition().coordinate());
            rwyIt->second.lastReportedPosition = flight.currentPosition().coordinate();
        }
    }
}

void DepartureSequenceControl::removeFlight(const std::string& callsign) {
    auto dIt = this->m_departureReady.find(callsign);
    if (this->m_departureReady.end() != dIt)
        this->m_departureReady.erase(dIt);

    for (auto it = this->m_departedPerRunway.begin(); this->m_departedPerRunway.end() != it; ++it) {
        if (it->second.callsign == callsign) {
            it->second.actualTakeOffTime = TimePoint();
            it->second.flewDistance = 0.0_m;
            it->second.callsign = "";
            break;
        }
    }
}

std::list<std::string> DepartureSequenceControl::allReadyForDepartureFlights() const {
    std::list<std::string> retval;

    for (auto it = this->m_departureReady.cbegin(); this->m_departureReady.cend() != it; ++it)
        retval.push_back(it->first);

    return std::move(retval);
}

bool DepartureSequenceControl::readyForDeparture(const types::Flight& flight) const {
    auto it = this->m_departureReady.find(flight.callsign());
    return this->m_departureReady.cend() != it;
}

void DepartureSequenceControl::departureSpacing(const types::Flight& flight, types::Time& timeSpacing, types::Length& separation) const {
    timeSpacing = 0.0_s;
    separation = 0.0_m;

    auto rwyIt = this->m_departedPerRunway.find(flight.flightPlan().departureRunway());
    if (this->m_departedPerRunway.cend() != rwyIt && 0 != rwyIt->second.callsign.length()) {
        auto id = std::make_pair(rwyIt->second.wtc, flight.flightPlan().aircraft().wtc());

        auto reqDistance = system::Separation::EuclideanDistance.find(id);
        auto reqTime = system::Separation::TimeDistance.find(id);
        if (system::Separation::EuclideanDistance.cend() == reqDistance || system::Separation::TimeDistance.cend() == reqTime)
            return;

        separation = reqDistance->second;
        separation -= rwyIt->second.flewDistance;
        if (0.0_m > separation)
            separation = 0.0_m;

        auto now = std::chrono::system_clock::now();
        timeSpacing = reqTime->second;
        timeSpacing -= static_cast<float>(std::chrono::duration_cast<std::chrono::seconds>(now - rwyIt->second.actualTakeOffTime).count()) * types::second;
        if (0.0_s > timeSpacing)
            timeSpacing = 0.0_s;
    }
}
