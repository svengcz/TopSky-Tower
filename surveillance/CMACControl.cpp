/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the Conformance Monitoring Alerts for Controllers System
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <GeographicLib/Gnomonic.hpp>

#include <surveillance/CMACControl.h>
#include <system/ConfigurationRegistry.h>

using namespace topskytower;
using namespace topskytower::surveillance;
using namespace topskytower::types;

CMACControl::CMACControl(const std::string& airport, const types::Coordinate& center) :
        m_holdingPoints(airport, center),
        m_tracks() {
    system::ConfigurationRegistry::instance().registerNotificationCallback(this, &CMACControl::reinitialize);

    this->reinitialize(system::ConfigurationRegistry::UpdateType::All);
}

CMACControl::~CMACControl() {
    system::ConfigurationRegistry::instance().deleteNotificationCallback(this);
}

void CMACControl::reinitialize(system::ConfigurationRegistry::UpdateType type) {
    if (system::ConfigurationRegistry::UpdateType::All != type && system::ConfigurationRegistry::UpdateType::Airports != type)
        return;

    this->m_holdingPoints.reinitialize();
}

static __inline types::Angle __normalize(const types::Angle& angle) {
    auto retval(angle);

    while (180.0_deg < retval)
        retval -= 360.0_deg;
    while (-1.0f * 180.0_deg > retval)
        retval += 360.0_deg;

    return retval;
}

void CMACControl::updateFlight(const types::Flight& flight, types::Flight::Type type) {
    /* check if the system is active */
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().cmacActive ||
        false == system::ConfigurationRegistry::instance().runtimeConfiguration().cmacActive)
    {
        return;
    }

    if (40.0_kn < flight.groundSpeed()) {
        this->removeFlight(flight.callsign());
        return;
    }

    auto it = this->m_tracks.find(flight.callsign());

    if (this->m_tracks.end() == it) {
        this->m_tracks[flight.callsign()] = FlightHistory();
        it = this->m_tracks.find(flight.callsign());

        it->second.expectedCommand = types::FlightPlan::AtcCommand::Unknown;
        it->second.behindHoldingPoint = false;
        it->second.cycleCounter = 0;
        it->second.referencePosition = flight.currentPosition().coordinate();

        return;
    }

    const auto& config = system::ConfigurationRegistry::instance().systemConfiguration();

    /* check if we have to increase the cycle counter */
    if (true == helper::Math::almostEqual(0.0f, flight.groundSpeed().value())) {
        it->second.cycleCounter += 1;

        /* check if we need to reset the internal data */
        if (config.cmacCycleReset < it->second.cycleCounter) {
            it->second.cycleCounter = 0;
            it->second.expectedCommand = types::FlightPlan::AtcCommand::Unknown;
            it->second.referencePosition = flight.currentPosition().coordinate();
        }

        return;
    }
    else {
        it->second.cycleCounter = 0;
    }

    /* check if we moved far enough to check the parameters */
    auto distance = it->second.referencePosition.distanceTo(flight.currentPosition().coordinate());
    if (config.cmacMinimumDistance > distance)
        return;

    /* get the difference between the current heading and the heading between the old and new position */
    auto heading = it->second.referencePosition.bearingTo(flight.currentPosition().coordinate());
    auto delta = heading - flight.currentPosition().heading();

    /* normalize the delta */
    while (0.0_deg > delta)
        delta += 360.0_deg;
    while (360.0_deg < delta)
        delta -= 360.0_deg;

    /* compare the expected with the current ATC command */
    if (types::Flight::Type::Departure == type) {
        if (90.0_deg < delta && 270.0_deg > delta)
            it->second.expectedCommand = types::FlightPlan::AtcCommand::Pushback;
        else
            it->second.expectedCommand = types::FlightPlan::AtcCommand::TaxiOut;
    }
    /* check if the flight crossed a runway exit */
    else if (false == it->second.behindHoldingPoint) {
        if (true == this->m_holdingPoints.passedHoldingPoint(flight, type, false, 0.0_m, 30.0_deg, nullptr) || 0.0_kn == flight.groundSpeed()) {
            it->second.expectedCommand = types::FlightPlan::AtcCommand::TaxiIn;
            it->second.behindHoldingPoint = true;
        }
        else {
            it->second.expectedCommand = types::FlightPlan::AtcCommand::Land;
        }
    }
    /* already left the runway */
    else {
        it->second.expectedCommand = types::FlightPlan::AtcCommand::TaxiIn;
    }

    it->second.referencePosition = flight.currentPosition().coordinate();
}

void CMACControl::removeFlight(const std::string& callsign) {
    auto it = this->m_tracks.find(callsign);
    if (this->m_tracks.end() != it)
        this->m_tracks.erase(it);
}

bool CMACControl::conformanceMonitoringAlert(const types::Flight& flight, types::Flight::Type type) const {
    /* check if the system is active */
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().cmacActive ||
        false == system::ConfigurationRegistry::instance().runtimeConfiguration().cmacActive)
    {
        return false;
    }

    auto it = this->m_tracks.find(flight.callsign());

    if (this->m_tracks.cend() != it && types::FlightPlan::AtcCommand::Unknown != it->second.expectedCommand) {
        if (types::Flight::Type::Departure == type) {
            if (types::FlightPlan::AtcCommand::Pushback == it->second.expectedCommand)
                return flight.flightPlan().departureFlag() != it->second.expectedCommand;
            else
                return flight.flightPlan().departureFlag() < it->second.expectedCommand;
        }
        else {
            return flight.flightPlan().arrivalFlag() != it->second.expectedCommand;
        }
    }

    return false;
}
