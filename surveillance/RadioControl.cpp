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

void RadioControl::transmissions(const std::vector<std::string>& callsigns) {
    std::lock_guard guard(this->m_transmissionsLock);

    this->m_activeTransmissions.clear();
    for (auto& callsign : callsigns) {
        if (true == system::FlightRegistry::instance().flightExists(callsign))
            this->m_activeTransmissions.push_back(std::move(callsign));
    }
}

bool RadioControl::isTransmitting(const types::Flight& flight) {
    std::lock_guard guard(this->m_transmissionsLock);

    for (const auto& transmission : std::as_const(this->m_activeTransmissions)) {
        if (transmission == flight.callsign())
            return true;
    }

    return false;
}

std::list<std::string> RadioControl::transmittingFlights() {
    std::list<std::string> retval;

    std::lock_guard guard(this->m_transmissionsLock);
    retval = this->m_activeTransmissions;

    return retval;
}

RadioControl& RadioControl::instance() {
    static RadioControl __instance;
    return __instance;
}
