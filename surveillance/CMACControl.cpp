/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the Autonomous Runway Incursion Warning System
 */

#include <GeographicLib/Gnomonic.hpp>

#include <surveillance/CMACControl.h>
#include <system/ConfigurationRegistry.h>

using namespace topskytower;
using namespace topskytower::surveillance;
using namespace topskytower::types;

CMACControl::CMACControl() :
        m_tracks() { }

void CMACControl::updateFlight(const types::Flight& flight) {
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
    if (types::Flight::Type::Departure == flight.type()) {
        if (90.0_deg < delta && 270.0_deg > delta)
            it->second.expectedCommand = types::FlightPlan::AtcCommand::Pushback;
        else
            it->second.expectedCommand = types::FlightPlan::AtcCommand::TaxiOut;
    }
    else {
        it->second.expectedCommand = types::FlightPlan::AtcCommand::TaxiIn;
    }
}

void CMACControl::removeFlight(const std::string& callsign) {
    auto it = this->m_tracks.find(callsign);
    if (this->m_tracks.end() != it)
        this->m_tracks.erase(it);
}

bool CMACControl::conformanceMonitoringAlert(const types::Flight& flight) const {
    /* check if the system is active */
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().cmacActive ||
        false == system::ConfigurationRegistry::instance().runtimeConfiguration().cmacActive)
    {
        return false;
    }

    auto it = this->m_tracks.find(flight.callsign());

    if (this->m_tracks.cend() != it) {
        if (types::Flight::Type::Departure == flight.type()) {
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