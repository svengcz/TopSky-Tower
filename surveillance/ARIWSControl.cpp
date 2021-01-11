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
        m_airportIcao(airport),
        m_centerPosition(center),
        m_normalHoldingPointTree(),
        m_normalHoldingPointTreeAdaptor(nullptr),
        m_lvpHoldingPointTree(),
        m_lvpHoldingPointTreeAdaptor(nullptr),
        m_incursionWarnings() {
    system::ConfigurationRegistry::instance().registerNotificationCallback(this, &ARIWSControl::reinitialize);

    this->reinitialize(system::ConfigurationRegistry::UpdateType::All);
}

ARIWSControl::~ARIWSControl() {
    system::ConfigurationRegistry::instance().deleteNotificationCallback(this);

    if (nullptr != this->m_normalHoldingPointTreeAdaptor)
        delete this->m_normalHoldingPointTreeAdaptor;
    this->m_normalHoldingPointTreeAdaptor = nullptr;
    this->m_normalHoldingPointTree.holdingPoints.clear();

    if (nullptr != this->m_lvpHoldingPointTreeAdaptor)
        delete this->m_lvpHoldingPointTreeAdaptor;
    this->m_lvpHoldingPointTreeAdaptor = nullptr;
    this->m_lvpHoldingPointTree.holdingPoints.clear();
}

void ARIWSControl::copyHoldingPointData(const types::HoldingPoint& config, HoldingPointData& data) {
    data.holdingPoint = config.holdingPoint;
    data.lowVisibility = config.lowVisibility;
    data.heading = config.heading;
    data.runway = config.runway;
}

void ARIWSControl::reinitialize(system::ConfigurationRegistry::UpdateType type) {
    if (system::ConfigurationRegistry::UpdateType::All != type && system::ConfigurationRegistry::UpdateType::Airports != type)
        return;

    /* delete all old information */
    if (nullptr != this->m_normalHoldingPointTreeAdaptor)
        delete this->m_normalHoldingPointTreeAdaptor;
    this->m_normalHoldingPointTreeAdaptor = nullptr;
    if (nullptr != this->m_lvpHoldingPointTreeAdaptor)
        delete this->m_lvpHoldingPointTreeAdaptor;
    this->m_lvpHoldingPointTreeAdaptor = nullptr;

    /* keep the old holding point informations to copy them into the new structures */
    auto oldNormalHoldingPoints = std::move(this->m_normalHoldingPointTree.holdingPoints);
    auto oldLvpHoldingPoints = std::move(this->m_lvpHoldingPointTree.holdingPoints);

    const auto& config = system::ConfigurationRegistry::instance().airportConfiguration(this->m_airportIcao);
    if (false == config.valid || 0 == config.aircraftStands.size())
        return;

    /* copy the stand data into the new structure and convert the coordinate to Cartesian coordinates */
    this->m_normalHoldingPointTree.holdingPoints.reserve(config.holdingPoints.size());
    this->m_lvpHoldingPointTree.holdingPoints.reserve(config.holdingPoints.size());

    GeographicLib::Gnomonic projection(GeographicLib::Geodesic::WGS84());
    for (const auto& holdingPoint : std::as_const(config.holdingPoints)) {
        HoldingPointData data;
        ARIWSControl::copyHoldingPointData(holdingPoint, data);

        projection.Forward(this->m_centerPosition.latitude().convert(types::degree),
                           this->m_centerPosition.longitude().convert(types::degree),
                           data.holdingPoint.latitude().convert(types::degree),
                           data.holdingPoint.longitude().convert(types::degree),
                           data.cartesianPosition[0],
                           data.cartesianPosition[1]);

        if (true == data.lowVisibility)
            this->m_lvpHoldingPointTree.holdingPoints.push_back(std::move(data));
        else
            this->m_normalHoldingPointTree.holdingPoints.push_back(std::move(data));
    }

#pragma warning(disable: 4127)
    this->m_normalHoldingPointTreeAdaptor = new HoldingPointTreeAdaptor(2, this->m_normalHoldingPointTree,
                                                                        nanoflann::KDTreeSingleIndexAdaptorParams(10));
    this->m_lvpHoldingPointTreeAdaptor = new HoldingPointTreeAdaptor(2, this->m_lvpHoldingPointTree,
                                                                     nanoflann::KDTreeSingleIndexAdaptorParams(10));
#pragma warning(default: 4127)
    this->m_normalHoldingPointTreeAdaptor->buildIndex();
    this->m_lvpHoldingPointTreeAdaptor->buildIndex();
}

void ARIWSControl::updateFlight(const types::Flight& flight) {
    /* check if the system is active */
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().ariwsActive)
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

    /* get the correct adaptor */
    HoldingPointTreeAdaptor* adaptor;
    if (false == system::ConfigurationRegistry::instance().runtimeConfiguration().lowVisibilityProcedures)
        adaptor = this->m_normalHoldingPointTreeAdaptor;
    else
        adaptor = this->m_lvpHoldingPointTreeAdaptor;

    /* project to Cartesian coordinates */
    GeographicLib::Gnomonic projection(GeographicLib::Geodesic::WGS84());
    float queryPt[2];
    projection.Forward(this->m_centerPosition.latitude().convert(types::degree),
                       this->m_centerPosition.longitude().convert(types::degree),
                       flight.currentPosition().coordinate().latitude().convert(types::degree),
                       flight.currentPosition().coordinate().longitude().convert(types::degree),
                       queryPt[0], queryPt[1]);

    std::size_t idx;
    float distance;

    /* something went wrong in the search-call */
    if (1 != adaptor->knnSearch(queryPt, 1, &idx, &distance))
        return;

    types::Angle holdingPointHeading;
    types::Coordinate holdingPoint;
    if (false == system::ConfigurationRegistry::instance().runtimeConfiguration().lowVisibilityProcedures) {
        holdingPointHeading = this->m_normalHoldingPointTree.holdingPoints[idx].heading;
        holdingPoint = this->m_normalHoldingPointTree.holdingPoints[idx].holdingPoint;
    }
    else {
        holdingPointHeading = this->m_lvpHoldingPointTree.holdingPoints[idx].heading;
        holdingPoint = this->m_lvpHoldingPointTree.holdingPoints[idx].holdingPoint;
    }

    /* the heading must be comparable */
    if (15_deg < (holdingPointHeading - flight.currentPosition().heading()).abs())
        return;

    auto distHP = holdingPoint.distanceTo(flight.currentPosition().coordinate());
    if (system::ConfigurationRegistry::instance().systemConfiguration().ariwsMaximumDistance >= distHP) {
        /* check if the holding point is behind the flight */
        auto heading = flight.currentPosition().coordinate().bearingTo(holdingPoint);
        if ((heading - flight.currentPosition().heading() - 180.0_deg).abs() < 15_deg) {
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
    if (false == system::ConfigurationRegistry::instance().systemConfiguration().ariwsActive)
        return false;

    auto aIt = std::find(this->m_incursionWarnings.begin(), this->m_incursionWarnings.end(), flight.callsign());
    return this->m_incursionWarnings.end() != aIt;
}
