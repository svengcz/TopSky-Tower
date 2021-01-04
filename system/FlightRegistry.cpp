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
    this->m_flights[callsign] = flight;
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

FlightRegistry& FlightRegistry::instance() {
    static FlightRegistry __instance;
    return __instance;
}