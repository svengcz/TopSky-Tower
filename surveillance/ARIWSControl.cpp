/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the Autonomous Runway Incursion Warning System
 */

#include <GeographicLib/Gnomonic.hpp>

#include <surveillance/ARIWSControl.h>
#include <system/ConfigurationRegistry.h>

using namespace topskytower;
using namespace topskytower::surveillance;
using namespace topskytower::types;

ARIWSControl::ARIWSControl(const std::string& airport, const types::Coordinate& center) :
        management::HoldingPointMap<management::HoldingPointData>(airport, center),
        m_incursionWarnings() {
    system::ConfigurationRegistry::instance().registerNotificationCallback(this, &ARIWSControl::reinitialize);

    this->reinitialize(system::ConfigurationRegistry::UpdateType::All);
}

ARIWSControl::~ARIWSControl() {
    system::ConfigurationRegistry::instance().deleteNotificationCallback(this);
}

void ARIWSControl::reinitialize(system::ConfigurationRegistry::UpdateType type) {
    if (system::ConfigurationRegistry::UpdateType::All != type && system::ConfigurationRegistry::UpdateType::Airports != type)
        return;

    management::HoldingPointMap<management::HoldingPointData>::reinitialize();
}

static __inline types::Angle __normalize(const types::Angle& angle) {
    auto retval(angle);

    while (180.0_deg < retval)
        retval -= 360.0_deg;
    while (-1.0f * 180.0_deg > retval)
        retval += 360.0_deg;

    return retval;
}

void ARIWSControl::updateFlight(const types::Flight& flight) {
    /* check if the system is active */
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().ariwsActive ||
        false == system::ConfigurationRegistry::instance().runtimeConfiguration().ariwsActive)
    {
        return;
    }

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

    /* find the next holding point */
    auto node = this->findNextHoldingPoint(flight);
    if (nullptr == node)
        return;

    auto holdingPointHeading = node->heading;
    auto holdingPoint = node->holdingPoint;

    /* the heading must be comparable */
    if (15_deg < __normalize(holdingPointHeading - flight.currentPosition().heading()).abs())
        return;

    auto distHP = holdingPoint.distanceTo(flight.currentPosition().coordinate());
    if (system::ConfigurationRegistry::instance().systemConfiguration().ariwsMaximumDistance >= distHP) {
        /* check if the holding point is behind the flight */
        auto heading = flight.currentPosition().coordinate().bearingTo(holdingPoint);
        if (__normalize(heading - flight.currentPosition().heading() - 180.0_deg).abs() < 15_deg) {
            /* check if the flight is behind the deadband */
            if (distHP > system::ConfigurationRegistry::instance().systemConfiguration().ariwsDistanceDeadband)
                this->m_incursionWarnings.push_back(flight.callsign());
        }
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
