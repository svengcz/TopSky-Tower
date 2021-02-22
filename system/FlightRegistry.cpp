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
        /* track the flags of the last update */
        auto depFlags = it->second.first.flightPlan().departureFlag();
        auto arrFlags = it->second.first.flightPlan().arrivalFlag();
        bool airborne = it->second.first.airborne();

        /* update the flight information */
        it->second.first = flight;

        /* update the internal flags, if needed */
        it->second.first.setAirborne(true == airborne ? true : it->second.first.airborne());

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

            if (types::FlightPlan::AtcCommand::StartUp == newFlag && types::FlightPlan::AtcCommand::Unknown == depFlags)
                it->second.first.flightPlan().resetFlag(true);
            else
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

void FlightRegistry::setAtcClearanceFlag(const types::Flight& flight, std::uint16_t flag) {
    types::FlightPlan::AtcCommand departure = static_cast<types::FlightPlan::AtcCommand>(flag & 0x0ff);
    types::FlightPlan::AtcCommand arrival = static_cast<types::FlightPlan::AtcCommand>(flag & 0xf00);
    auto it = this->m_flights.find(flight.callsign());

    if (this->m_flights.end() == it)
        return;

    switch (departure) {
    case types::FlightPlan::AtcCommand::Unknown:
        if (types::FlightPlan::AtcCommand::Unknown != it->second.second)
            it->second.second = types::FlightPlan::AtcCommand::StartUp;
        break;
    case types::FlightPlan::AtcCommand::StartUp:
        it->second.second = types::FlightPlan::AtcCommand::StartUp;
        break;
    case types::FlightPlan::AtcCommand::Pushback:
        it->second.second = types::FlightPlan::AtcCommand::Pushback;
        break;
    case types::FlightPlan::AtcCommand::TaxiOut:
    case types::FlightPlan::AtcCommand::LineUp:
        it->second.second = types::FlightPlan::AtcCommand::TaxiOut;
        break;
    case types::FlightPlan::AtcCommand::Departure:
        it->second.second = types::FlightPlan::AtcCommand::Departure;
        break;
    case types::FlightPlan::AtcCommand::Deicing:
    default:
        break;
    }

    /* update the departure status */
    if (types::FlightPlan::AtcCommand::Unknown == departure)
        it->second.first.flightPlan().resetFlag(true);
    else
        it->second.first.flightPlan().setFlag(departure);

    /* update the arrival status */
    if (types::FlightPlan::AtcCommand::Unknown == arrival)
        it->second.first.flightPlan().resetFlag(false);
    else
        it->second.first.flightPlan().setFlag(arrival);
}

FlightRegistry& FlightRegistry::instance() {
    static FlightRegistry __instance;
    return __instance;
}
