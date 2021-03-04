/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the Autonomous Runway Incursion Warning System
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <management/NotamControl.h>
#include <surveillance/ARIWSControl.h>
#include <system/ConfigurationRegistry.h>

using namespace topskytower;
using namespace topskytower::surveillance;
using namespace topskytower::types;

ARIWSControl::ARIWSControl(const std::string& airport, const types::Coordinate& center) :
        m_airportIcao(airport),
        m_holdingPoints(airport, center),
        m_incursionWarnings(),
        m_inactiveRunways() {
    system::ConfigurationRegistry::instance().registerNotificationCallback(this, &ARIWSControl::reinitialize);
    management::NotamControl::instance().registerNotificationCallback(this, &ARIWSControl::notamsChanged);

    this->reinitialize(system::ConfigurationRegistry::UpdateType::All);
}

ARIWSControl::~ARIWSControl() {
    system::ConfigurationRegistry::instance().deleteNotificationCallback(this);
    management::NotamControl::instance().deleteNotificationCallback(this);
}

void ARIWSControl::reinitialize(system::ConfigurationRegistry::UpdateType type) {
    if (system::ConfigurationRegistry::UpdateType::All != type && system::ConfigurationRegistry::UpdateType::Airports != type)
        return;

    this->m_holdingPoints.reinitialize();
}

void ARIWSControl::notamsChanged() {
    auto notams = management::NotamControl::instance().notams(this->m_airportIcao, management::NotamCategory::Runway);

    this->m_inactiveRunways.clear();
    for (const auto& notam : std::as_const(notams)) {
        if (management::NotamInterpreterState::Success != notam->interpreterState)
            continue;
        if (false == notam->active())
            continue;

        const auto& runways = static_cast<management::RunwayNotam*>(notam.get())->sections;
        for (const auto& runway : std::as_const(runways)) {
            auto it = std::find(this->m_inactiveRunways.cbegin(), this->m_inactiveRunways.cend(), runway);
            if (this->m_inactiveRunways.cend() == it)
                this->m_inactiveRunways.push_back(runway);
        }
    }
}

void ARIWSControl::updateFlight(const types::Flight& flight, types::Flight::Type type) {
    /* check if the system is active */
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().ariwsActive ||
        false == system::ConfigurationRegistry::instance().runtimeConfiguration().ariwsActive)
    {
        return;
    }

    /* ignore non-departing traffic */
    if (types::Flight::Type::Departure != type)
        return;

    /* ignore departing or lining up flights */
    auto aIt = std::find(this->m_incursionWarnings.begin(), this->m_incursionWarnings.end(), flight.callsign());
    if (types::FlightPlan::AtcCommand::LineUp == flight.flightPlan().departureFlag() ||
        types::FlightPlan::AtcCommand::Departure == flight.flightPlan().departureFlag() ||
        40_kn < flight.groundSpeed())
    {
        this->removeFlight(flight.callsign());
        return;
    }
    /* check if the flight is marked as RIW */
    else if (this->m_incursionWarnings.end() != aIt) {
        return;
    }

    std::size_t index;
    auto deadband = system::ConfigurationRegistry::instance().systemConfiguration().ariwsDistanceDeadband;
    if (true == this->m_holdingPoints.passedHoldingPoint(flight, type, false, deadband, 15.0_deg, &index)) {
        auto point = this->m_holdingPoints.holdingPoint(system::ConfigurationRegistry::instance().runtimeConfiguration().lowVisibilityProcedures, index);
        auto it = std::find(this->m_inactiveRunways.cbegin(), this->m_inactiveRunways.cend(), point.runway);
        if (this->m_inactiveRunways.cend() == it)
            this->m_incursionWarnings.push_back(flight.callsign());
    }
}

void ARIWSControl::removeFlight(const std::string& callsign) {
    auto aIt = std::find(this->m_incursionWarnings.begin(), this->m_incursionWarnings.end(), callsign);
    if (this->m_incursionWarnings.end() != aIt)
        this->m_incursionWarnings.erase(aIt);
}

bool ARIWSControl::runwayIncursionWarning(const types::Flight& flight) const {
    /* check if the system is active */
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().ariwsActive ||
        false == system::ConfigurationRegistry::instance().runtimeConfiguration().ariwsActive)
    {
        return false;
    }

    auto aIt = std::find(this->m_incursionWarnings.begin(), this->m_incursionWarnings.end(), flight.callsign());
    return this->m_incursionWarnings.end() != aIt;
}
