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

#pragma warning(disable: 5054)
#include <Eigen/Core>
#include <Eigen/Geometry>
#pragma warning(default:5054)
#include <GeographicLib/Gnomonic.hpp>
#include <GeographicLib/LocalCartesian.hpp>

#include <surveillance/STCDControl.h>

using namespace topskytower;
using namespace topskytower::surveillance;
using namespace topskytower::types;

static const std::map<std::pair<types::Aircraft::WTC, types::Aircraft::WTC>, types::Length> __distances = {
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Unknown), 3_nm },
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Light),   3_nm },
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Medium),  3_nm },
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Heavy),   3_nm },
    { std::make_pair(types::Aircraft::WTC::Unknown, types::Aircraft::WTC::Super),   3_nm },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Unknown), 3_nm },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Light),   3_nm },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Medium),  3_nm },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Heavy),   3_nm },
    { std::make_pair(types::Aircraft::WTC::Light,   types::Aircraft::WTC::Super),   3_nm },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Unknown), 3_nm },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Light),   5_nm },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Medium),  3_nm },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Heavy),   3_nm },
    { std::make_pair(types::Aircraft::WTC::Medium,  types::Aircraft::WTC::Super),   3_nm },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Unknown), 4_nm },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Light),   6_nm },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Medium),  5_nm },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Heavy),   4_nm },
    { std::make_pair(types::Aircraft::WTC::Heavy,   types::Aircraft::WTC::Super),   4_nm },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Unknown), 6_nm },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Light),   8_nm },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Medium),  7_nm },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Heavy),   6_nm },
    { std::make_pair(types::Aircraft::WTC::Super,   types::Aircraft::WTC::Super),   6_nm }
};

STCDControl::STCDControl(const std::string& airport, const types::Length& elevation, const types::Coordinate& center,
                         const std::list<types::Runway>& runways) :
        m_airportIcao(airport),
        m_airportElevation(elevation + 100_ft),
        m_reference(center),
        m_holdingPoints(airport, center),
        m_runways(runways),
        m_noTransgressionZones(),
        m_ntzViolations(),
        m_inbounds(),
        m_conflicts() {
    system::ConfigurationRegistry::instance().registerNotificationCallback(this, &STCDControl::reinitialize);

    this->reinitialize(system::ConfigurationRegistry::UpdateType::All);
}

STCDControl::~STCDControl() {
    system::ConfigurationRegistry::instance().deleteNotificationCallback(this);
}

void STCDControl::createNTZ(const std::pair<std::string, std::string>& runwayPair) {
    types::Runway rwy[2];

    /* find the corresponding runways */
    for (const auto& runway : std::as_const(this->m_runways)) {
        if (runway.name() == runwayPair.first)
            rwy[0] = runway;
        else if (runway.name() == runwayPair.second)
            rwy[1] = runway;
    }

    /* validate that both runways are found */
    if (0 == rwy[0].name().length() || 0 == rwy[1].name().length())
        return;

    /* calculate the heading between both runways */
    auto ntzHeading = (rwy[0].end().bearingTo(rwy[0].start()) + rwy[1].end().bearingTo(rwy[1].start())) * 0.5f;

    GeographicLib::LocalCartesian projection(this->m_reference.latitude().convert(types::degree),
                                             this->m_reference.longitude().convert(types::degree));

    /* calculate the center between both runways */
    Eigen::Vector3f gnonomicThresholds[2];
    projection.Forward(rwy[0].start().latitude().convert(types::degree), rwy[0].start().longitude().convert(types::degree), 0.0f,
                       gnonomicThresholds[0][0], gnonomicThresholds[0][1], gnonomicThresholds[0][2]);
    projection.Forward(rwy[1].start().latitude().convert(types::degree), rwy[1].start().longitude().convert(types::degree), 0.0f,
                       gnonomicThresholds[1][0], gnonomicThresholds[1][1], gnonomicThresholds[1][2]);
    Eigen::Vector3f gnonomicRunwayCenter = gnonomicThresholds[0] + 0.5f * (gnonomicThresholds[1] - gnonomicThresholds[0]);

    /* find a point on the center line in the direction of the NTZ to define the center line */
    float coordinates[3];
    projection.Reverse(gnonomicRunwayCenter[0], gnonomicRunwayCenter[1], gnonomicRunwayCenter[2],
                       coordinates[0], coordinates[1], coordinates[2]);
    types::Coordinate ntzStart(coordinates[1] * types::degree, coordinates[0] * types::degree);

    types::Coordinate ntzEnd = ntzStart.projection(ntzHeading, 10_nm);
    Eigen::Vector3f gnonomicNtzPosition;
    projection.Forward(ntzEnd.latitude().convert(types::degree), ntzEnd.longitude().convert(types::degree), 0.0f,
                       gnonomicNtzPosition[0], gnonomicNtzPosition[1], gnonomicNtzPosition[2]);

    /* define the NTZ-center line to project the points of the runway */
    Eigen::ParametrizedLine<float, 3> ntzLine(gnonomicRunwayCenter, (gnonomicNtzPosition - gnonomicRunwayCenter).normalized());

    /* find the correct starting point of the NTZ (beginning of the "earlier" runway threshold) */
    auto projectionRwy0 = ntzLine.projection(gnonomicThresholds[0]);
    auto projectionRwy1 = ntzLine.projection(gnonomicThresholds[1]);
    auto projectionDirection = (projectionRwy1 - projectionRwy0).normalized();
    if (0.1f >= (ntzLine.direction() - projectionDirection).norm()) {
        projection.Reverse(projectionRwy1[0], projectionRwy1[1], projectionRwy1[2],
                           coordinates[0], coordinates[1], coordinates[2]);
    }
    else {
        projection.Reverse(projectionRwy0[0], projectionRwy0[1], projectionRwy0[2],
                           coordinates[0], coordinates[1], coordinates[2]);
    }
    ntzStart = types::Coordinate(coordinates[1] * types::degree, coordinates[0] * types::degree);

    /* calculate the NTZ's end point */
    ntzEnd = ntzStart.projection(ntzHeading, 10_nm);

    /* calculate the edges in a fixed order (the required polygon-order will be fixed in SectorBorder, if needed) */
    std::list<types::Coordinate> edges;
    edges.push_back(ntzEnd.projection(rwy[0].heading() + 90.0_deg, 1000_ft));
    edges.push_back(ntzStart.projection(rwy[1].heading() + 90.0_deg, 1000_ft));
    edges.push_back(ntzStart.projection(rwy[1].heading() - 90.0_deg, 1000_ft));
    edges.push_back(ntzEnd.projection(rwy[0].heading() - 90.0_deg, 1000_ft));

    /* create the border */
    types::SectorBorder ntz("", {}, 0_ft, 99000_ft);
    ntz.setEdges(edges);
    this->m_noTransgressionZones.push_back(std::move(ntz));
}

void STCDControl::reinitialize(system::ConfigurationRegistry::UpdateType type) {
    if (system::ConfigurationRegistry::UpdateType::All != type && system::ConfigurationRegistry::UpdateType::Runtime != type)
        return;

    this->m_holdingPoints.reinitialize();

    const auto& configuration = system::ConfigurationRegistry::instance().runtimeConfiguration();
    this->m_noTransgressionZones.clear();

    /* no IPA active -> no NTZ-definition needed */
    if (false == configuration.ipaActive) {
        this->m_ntzViolations.clear();
        return;
    }

    std::list<std::pair<std::string, std::string>> ipaPairs, prmPairs;

    const auto& airportConfig = system::ConfigurationRegistry::instance().airportConfiguration(this->m_airportIcao);

    /* no active arrival runways found configuration found */
    if (configuration.activeArrivalRunways.cend() == configuration.activeArrivalRunways.find(this->m_airportIcao))
        return;

    /* get the IPA and PRM pairs that are possible based on the static and runtime configuration */
    const auto& arrivalRunways = configuration.activeArrivalRunways.find(this->m_airportIcao)->second;
    for (auto it = arrivalRunways.cbegin(); arrivalRunways.cend() != it; ++it) {
        auto cit = it;
        std::advance(cit, 1);

        /* check if IPA is available for this runway */
        if (airportConfig.ipaRunways.cend() != airportConfig.ipaRunways.find(*it)) {
            const auto& partnerRwys = airportConfig.ipaRunways.find(*it)->second;

            for (; arrivalRunways.cend() != cit; ++cit) {
                auto partner = std::find(partnerRwys.cbegin(), partnerRwys.cend(), *cit);
                if (partnerRwys.cend() != partner)
                    ipaPairs.push_back(std::make_pair(*it, *cit));
            }
        }

        /* check if PRM is available for this runway */
        if (airportConfig.prmRunways.cend() != airportConfig.prmRunways.find(*it)) {
            const auto& partnerRwys = airportConfig.prmRunways.find(*it)->second;

            for (; arrivalRunways.cend() != cit; ++cit) {
                auto partner = std::find(partnerRwys.cbegin(), partnerRwys.cend(), *cit);
                if (partnerRwys.cend() != partner)
                    prmPairs.push_back(std::make_pair(*it, *cit));
            }
        }
    }

    /* create all required NTZs */
    for (const auto& pair : std::as_const(ipaPairs))
        this->createNTZ(pair);
    for (const auto& pair : std::as_const(prmPairs))
        this->createNTZ(pair);
}

static __inline void __normalizeAngle(types::Angle& angle) {
    while (-1.0f * 180.0_deg > angle)
        angle += 360.0_deg;
    while (180.0_deg < angle)
        angle -= 360.0_deg;
}

void STCDControl::analyzeInbound(const types::Flight& flight) {
    /* flight violated NTZ -> has to go around */
    auto ntzViolationIt = std::find(this->m_ntzViolations.cbegin(), this->m_ntzViolations.cend(), flight.callsign());
    bool violatesNtz = this->m_ntzViolations.cend() != ntzViolationIt;

    this->removeFlight(flight.callsign());

    /* ignore landed or going around flights */
    bool landed = 40_kn > flight.groundSpeed() || this->m_airportElevation >= flight.currentPosition().altitude();
    if (true == landed || types::FlightPlan::AtcCommand::GoAround == flight.flightPlan().arrivalFlag())
        return;

    /* find the corresponding runway */
    types::Runway inboundRunway;
    for (const auto& runway : std::as_const(this->m_runways)) {
        if (runway.name() == flight.flightPlan().arrivalRunway()) {
            inboundRunway = runway;
            break;
        }
    }
    if (0 == inboundRunway.name().length())
        return;

    /* validate that the flight is close enough and on the correct heading */
    if (20_nm <= inboundRunway.start().distanceTo(flight.currentPosition().coordinate()))
        return;
    auto delta = flight.currentPosition().heading() - inboundRunway.heading();
    __normalizeAngle(delta);
    if (15_deg <= delta.abs())
        return;

    /* flight violated NTZ -> has to go around */
    if (true == violatesNtz) {
        this->m_ntzViolations.push_back(flight.callsign());
        return;
    }

    /* test if a flight is in the NTZ */
    for (const auto& ntz : std::as_const(this->m_noTransgressionZones)) {
        /* flight is inside the NTZ -> mark it and return */
        if (true == ntz.isInsideBorder(flight.currentPosition().coordinate())) {
            if (this->m_ntzViolations.cend() == ntzViolationIt)
                this->m_ntzViolations.push_back(flight.callsign());
            return;
        }
    }

    /* find nearest flight */
    const auto& config = system::ConfigurationRegistry::instance().runtimeConfiguration();
    types::Length minDistance = 50_nm;
    types::Aircraft::WTC neighborWtc;
    types::Position neighborPosition;
    std::string neighborCallsign;
    std::string neighborRunway;
    for (auto& inbound : std::as_const(this->m_inbounds)) {
        /* ignore neighboring flights */
        if (true == config.ipaActive && inbound.flightPlan().arrivalRunway() != flight.flightPlan().arrivalRunway())
            continue;

        /* validate that the candidate is in front of this flight */
        delta = flight.currentPosition().coordinate().bearingTo(inbound.currentPosition().coordinate());
        delta -= flight.currentPosition().heading();
        __normalizeAngle(delta);
        if (90_deg < delta.abs())
            continue;

        /* find the closest flight */
        auto distance = inbound.currentPosition().coordinate().distanceTo(flight.currentPosition().coordinate());
        if (distance <= minDistance) {
            neighborWtc = inbound.flightPlan().aircraft().wtc();
            neighborPosition = inbound.currentPosition();
            neighborCallsign = inbound.callsign();
            neighborRunway = inbound.flightPlan().arrivalRunway();
            minDistance = distance;
        }
    }

    /* find the minimum required distance */
    types::Length minRequiredDistance;
    if (neighborRunway != flight.flightPlan().arrivalRunway()) {
        minRequiredDistance = 3_nm;
    }
    else {
        auto id = std::make_pair(neighborWtc, flight.flightPlan().aircraft().wtc());
        minRequiredDistance = __distances.find(id)->second;
    }

    /* check the distance with the minDistance with minRequiredDistance */
    if (minDistance < minRequiredDistance)
        this->m_conflicts[flight.callsign()] = minRequiredDistance;

    this->m_inbounds.push_back(flight);
}

void STCDControl::analyzeOutbound(const types::Flight& flight) {
    auto closestFlightIt = this->m_inbounds.cend();
    types::Length minDistance = 999_nm;

    /* find the closest inbound to check if the spacing is too small */
    for (auto it = this->m_inbounds.cbegin(); this->m_inbounds.cend() != it; ++it) {
        const auto& config = system::ConfigurationRegistry::instance().airportConfiguration(this->m_airportIcao);

        /* check if the runways are independent */
        auto depIt = config.ipdRunways.find(flight.flightPlan().departureRunway());
        if (config.ipdRunways.cend() != depIt) {
            auto ipdIt = std::find(depIt->second.cbegin(), depIt->second.cend(), it->flightPlan().arrivalRunway());
            if (depIt->second.cend() != ipdIt)
                continue;
        }

        auto distance = it->currentPosition().coordinate().distanceTo(flight.currentPosition().coordinate());

        /* found a closer flight */
        if (minDistance > distance) {
            minDistance = distance;
            closestFlightIt = it;
        }
    }

    /* check if it is a conflict */
    if (this->m_inbounds.cend() != closestFlightIt) {
        auto id = std::make_pair(flight.flightPlan().aircraft().wtc(), closestFlightIt->flightPlan().aircraft().wtc());
        auto minRequiredDistance = __distances.find(id)->second;
        if (minRequiredDistance >= minDistance) {
            this->m_conflicts[flight.callsign()] = minRequiredDistance;
            return;
        }
    }

    this->removeFlight(flight.callsign());
}

void STCDControl::updateFlight(const types::Flight& flight, types::Flight::Type type) {
    if (false == system::ConfigurationRegistry::instance().runtimeConfiguration().stcdActive)
        return;

    /* we evaluate only the arrivals anf IFRs */
    if (types::FlightPlan::Type::IFR != flight.flightPlan().type())
        return;

    if (types::Flight::Type::Arrival == type)
        this->analyzeInbound(flight);
    else
        this->analyzeOutbound(flight);
}

void STCDControl::removeFlight(const std::string& callsign) {
    /* cleanup the NTZ violations */
    auto ntzViolationIt = std::find(this->m_ntzViolations.begin(), this->m_ntzViolations.end(), callsign);
    if (this->m_ntzViolations.end() != ntzViolationIt)
        this->m_ntzViolations.erase(ntzViolationIt);

    /* cleanup the inbounds */
    for (auto it = this->m_inbounds.begin(); this->m_inbounds.end() != it; ++it) {
        if (it->callsign() == callsign) {
            this->m_inbounds.erase(it);
            break;
        }
    }

    /* cleanup the conflicts */
    auto it = this->m_conflicts.find(callsign);
    if (this->m_conflicts.end() != it)
        this->m_conflicts.erase(it);
}

bool STCDControl::ntzViolation(const types::Flight& flight) const {
    auto it = std::find(this->m_ntzViolations.cbegin(), this->m_ntzViolations.cend(), flight.callsign());
    return this->m_ntzViolations.cend() != it;
}

bool STCDControl::separationLoss(const types::Flight& flight) const {
    auto it = this->m_conflicts.find(flight.callsign());
    return this->m_conflicts.cend() != it;
}

const types::Length& STCDControl::minSeparation(const types::Flight& flight) {
    static types::Length __fallback;

    auto it = this->m_conflicts.find(flight.callsign());
    if (this->m_conflicts.cend() != it)
        return it->second;
    else
        return __fallback;
}

const std::list<types::SectorBorder>& STCDControl::noTransgressionZones() const {
    return this->m_noTransgressionZones;
}
