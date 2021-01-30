/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the flight registry
 */

#include <system/FlightRegistry.h>

using namespace topskytower;
using namespace topskytower::system;

FlightRegistry::FlightRegistry() : m_flights() { }

void FlightRegistry::updateFlight(const types::Flight& flight) {
    std::string callsign(flight.callsign());
    auto it = this->m_flights.find(callsign);

    if (this->m_flights.end() != it) {
        auto depFlags = it->second.flightPlan().departureFlag();
        auto arrFlags = it->second.flightPlan().arrivalFlag();

        it->second = flight;

        /* keep the old status flags */
        it->second.flightPlan().setFlag(depFlags);
        it->second.flightPlan().setFlag(arrFlags);
    }
    else {
        this->m_flights[callsign] = flight;
    }
}

void FlightRegistry::updateGroundStatus(const types::Flight& flight) {
    auto it = this->m_flights.find(flight.callsign());

    if (this->m_flights.end() != it) {
        it->second.flightPlan().setFlag(flight.flightPlan().departureFlag());
        it->second.flightPlan().setFlag(flight.flightPlan().arrivalFlag());
    }
}

void FlightRegistry::removeFlight(const std::string& callsign) {
    auto it = this->m_flights.find(callsign);
    if (this->m_flights.end() != it)
        this->m_flights.erase(it);
}

bool FlightRegistry::flightExists(const std::string& callsign) const {
    return this->m_flights.cend() != this->m_flights.find(callsign);
}

const types::Flight& FlightRegistry::flight(const std::string& callsign) const {
    static types::Flight fallback;

    auto it = this->m_flights.find(callsign);
    if (this->m_flights.cend() != it)
        return it->second;
    else
        return fallback;
}
