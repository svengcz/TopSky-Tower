/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the flight registry
 */

#include <system/FlightRegistry.h>

using namespace topskytower;
using namespace topskytower::system;

static const std::map<types::FlightPlan::AtcCommand, std::vector<types::FlightPlan::AtcCommand>> __stsTranslation = {
    { types::FlightPlan::AtcCommand::StartUp,   { types::FlightPlan::AtcCommand::StartUp } },
    { types::FlightPlan::AtcCommand::Deicing,   { types::FlightPlan::AtcCommand::StartUp, types::FlightPlan::AtcCommand::TaxiOut } },
    { types::FlightPlan::AtcCommand::Pushback,  { types::FlightPlan::AtcCommand::Pushback } },
    { types::FlightPlan::AtcCommand::TaxiOut,   { types::FlightPlan::AtcCommand::TaxiOut } },
    { types::FlightPlan::AtcCommand::LineUp,    { types::FlightPlan::AtcCommand::TaxiOut } },
    { types::FlightPlan::AtcCommand::Departure, { types::FlightPlan::AtcCommand::Departure } }
};

FlightRegistry::FlightRegistry() : m_flights() { }

void FlightRegistry::updateFlight(const types::Flight& flight) {
    std::string callsign(flight.callsign());
    auto it = this->m_flights.find(callsign);

    if (this->m_flights.end() != it) {
        auto depFlags = it->second.flightPlan().departureFlag();
        auto arrFlags = it->second.flightPlan().arrivalFlag();

        it->second = flight;

        /* check if a normal ES controller updated the ground status */
        if (types::FlightPlan::AtcCommand::Unknown != depFlags) {
            const auto& table = __stsTranslation.find(depFlags);
            if (__stsTranslation.cend() != table) {
                auto allowed = std::find(table->second.cbegin(), table->second.cend(), flight.flightPlan().departureFlag());

                /* translate the entry and check if it is still consistent */
                if (table->second.cend() == allowed)
                    depFlags = flight.flightPlan().departureFlag();
            }
        }
        else {
            depFlags = flight.flightPlan().departureFlag();
        }

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
