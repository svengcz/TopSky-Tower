/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the flight registry
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <system/FlightRegistry.h>

using namespace topskytower;
using namespace topskytower::system;

FlightRegistry::FlightRegistry() : m_flights() { }

void FlightRegistry::updateFlight(const types::Flight& flight) {
    std::string callsign(flight.callsign());
    auto it = this->m_flights.find(callsign);

    if (this->m_flights.end() != it) {
        auto depFlags = it->second.first.flightPlan().departureFlag();
        auto arrFlags = it->second.first.flightPlan().arrivalFlag();
        bool airborne = it->second.first.airborne();

        it->second.first = flight;
        if (true == airborne)
            it->second.first.setAirborne(true);

        /* an update of the departure flag is possible */
        if (types::FlightPlan::AtcCommand::Unknown != flight.flightPlan().departureFlag()) {
            auto newFlag = flight.flightPlan().departureFlag();

            /* one of the ES standard flags is set */
            if (types::FlightPlan::AtcCommand::Deicing != newFlag && types::FlightPlan::AtcCommand::LineUp != newFlag) {
                /* the standard flag changed -> use the new flag and store it */
                if (it->second.second != newFlag) {
                    it->second.second = newFlag;
                    depFlags = newFlag;
                }
            }
            /* an GrPlugin- or TST-flag is set */
            else {
                depFlags = newFlag;
            }

            it->second.first.flightPlan().setFlag(depFlags);
        }
        /* restore the old entry */
        else if (types::FlightPlan::AtcCommand::Unknown != depFlags) {
            it->second.first.flightPlan().setFlag(depFlags);
        }

        /* no update of the arrival flag is possible -> restore the old status */
        if (types::FlightPlan::AtcCommand::Unknown == flight.flightPlan().arrivalFlag())
            it->second.first.flightPlan().setFlag(arrFlags);
    }
    else {
        this->m_flights[callsign] = std::make_pair(flight, flight.flightPlan().departureFlag());
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
        return it->second.first;
    else
        return fallback;
}

FlightRegistry& FlightRegistry::instance() {
    static FlightRegistry __instance;
    return __instance;
}
