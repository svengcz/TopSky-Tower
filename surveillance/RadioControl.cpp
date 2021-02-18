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

#include <surveillance/RadioControl.h>
#include <system/FlightRegistry.h>

using namespace topskytower;
using namespace topskytower::surveillance;

RadioControl::RadioControl() :
        m_transmissionsLock(),
        m_activeTransmissions() { }

void RadioControl::transmits(const std::string& callsign) {
    if (false == system::FlightRegistry::instance().flightExists(callsign))
        return;

    auto now = std::chrono::system_clock::now();
    std::lock_guard guard(this->m_transmissionsLock);

    for (auto& transmission : this->m_activeTransmissions) {
        if (transmission.callsign == callsign) {
            transmission.lastReceived = now;
            return;
        }
    }

    this->m_activeTransmissions.push_back({ callsign, now });
}

void RadioControl::timeout() {
    auto now = std::chrono::system_clock::now();

    std::lock_guard guard(this->m_transmissionsLock);

    for (auto it = this->m_activeTransmissions.begin(); this->m_activeTransmissions.end() != it;) {
        if (std::chrono::duration_cast<std::chrono::seconds>(now - it->lastReceived).count() >= 1)
            it = this->m_activeTransmissions.erase(it);
        else
            ++it;
    }
}

bool RadioControl::isTransmitting(const types::Flight& flight) {
    std::lock_guard guard(this->m_transmissionsLock);

    for (const auto& transmission : std::as_const(this->m_activeTransmissions)) {
        if (transmission.callsign == flight.callsign())
            return true;
    }

    return false;
}

std::list<std::string> RadioControl::transmittingFlights() {
    std::list<std::string> retval;

    std::lock_guard guard(this->m_transmissionsLock);

    for (const auto& transmission : std::as_const(this->m_activeTransmissions))
        retval.push_back(transmission.callsign);

    return std::move(retval);
}

RadioControl& RadioControl::instance() {
    static RadioControl __instance;
    return __instance;
}
