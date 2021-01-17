/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the departure model
 */

#include <GeographicLib/Gnomonic.hpp>

#include <surveillance/DepartureModel.h>
#include <system/ConfigurationRegistry.h>

using namespace topskytower;
using namespace topskytower::surveillance;
using namespace topskytower::types;

DepartureModel::DepartureModel(const std::string& callsign) :
        m_flight(types::Flight(callsign)),
        m_reference(),
        m_lastUpdate(),
        m_currentPhase(Phase::TakeOff),
        m_v2Speed(),
        m_climbRate(),
        m_climbRateAcceleration(),
        m_acceleration(),
        m_cruiseSpeed(),
        m_waypoints(),
        m_routeCartesian() { }

DepartureModel::DepartureModel(const types::Flight& flight, const types::Coordinate& reference,
                               const std::vector<types::Coordinate>& waypoints) :
        m_flight(flight),
        m_reference(reference),
        m_lastUpdate(std::chrono::system_clock::now()),
        m_currentPhase(Phase::TakeOff),
        m_v2Speed(),
        m_climbRate(),
        m_climbRateAcceleration(),
        m_acceleration(),
        m_cruiseSpeed(),
        m_waypoints(),
        m_routeCartesian() {
    const auto& config = system::ConfigurationRegistry::instance().systemConfiguration();

    int index = static_cast<int>(flight.flightPlan().aircraft().wtc());
    this->m_v2Speed = config.mtcdDepartureSpeedV2[index];
    this->m_climbRate = config.mtcdDepartureClimbRates[index];
    this->m_climbRateAcceleration = this->m_climbRate * 0.5f;
    this->m_acceleration = config.mtcdDepartureAcceleration;
    this->m_cruiseSpeed = config.mtcdDepartureCruiseTAS[index];

    this->m_currentPhase = this->identifyPhase(this->m_flight.currentPosition().altitude(), this->m_flight.groundSpeed(),
                                               this->m_flight.verticalSpeed());
    this->predictWaypoints(waypoints);
}

bool DepartureModel::operator==(const DepartureModel& other) const {
    return this->m_flight.callsign() == other.m_flight.callsign();
}

bool DepartureModel::operator!=(const DepartureModel& other) const {
    return false == this->operator==(other);
}

DepartureModel::Phase DepartureModel::identifyPhase(const types::Length& altitude, const types::Velocity& speed,
                                                    const types::Velocity& climbRate) const {
    const auto& config = system::ConfigurationRegistry::instance().systemConfiguration();

    /* we are in the acceleration phase or the take-off phase */
    if (altitude < config.mtcdDepartureAccelerationAlt) {
        /* we are below V2 or have a climb rate lower than 500 ft/min */
        if (speed < this->m_v2Speed && climbRate < 500_ftpmin)
            return Phase::TakeOff;
        else
            return Phase::AccelerationAltitude;
    }
    /* we are below FL100 */
    else if (altitude < 10000_ft && this->m_flight.flightPlan().flightLevel() >= 10000_ft) {
        /* we climb to FL100 */
        if (speed >= config.mtcdDepartureSpeedBelowFL100 * 0.95f || speed >= this->m_cruiseSpeed * 0.95f)
            return Phase::ClimbFL100;
        else
            return Phase::AccelerationFL100;
    }
    /* we are above FL100 */
    else {
        /* we climb to RFL */
        if (speed >= this->m_cruiseSpeed * 0.95f)
            return Phase::ClimbCruise;
        else
            return Phase::AccelerationCruise;
    }
}

types::Length DepartureModel::predictNextPhase(const types::Velocity& speed0, const types::Length& altitude0,
                                               Phase currentPhase, types::Velocity& speed1, types::Length& altitude1,
                                               types::Time& requiredTime) const {
    const auto& config = system::ConfigurationRegistry::instance().systemConfiguration();
    types::Length maxAltitude;
    types::Velocity maxSpeed;

    switch (currentPhase) {
    case Phase::TakeOff:
        if (this->m_v2Speed > speed0) {
            requiredTime = (this->m_v2Speed - speed0) / this->m_acceleration;
            altitude1 = altitude0;
            speed1 = this->m_v2Speed;
            return speed0 * requiredTime + 0.5f * this->m_acceleration * requiredTime * requiredTime;
        }
        break;
    case Phase::AccelerationAltitude:
        if (config.mtcdDepartureAccelerationAlt > altitude0) {
            requiredTime = (config.mtcdDepartureAccelerationAlt - altitude0) / (1.5f * this->m_climbRate);
            altitude1 = config.mtcdDepartureAccelerationAlt;
            speed1 = speed0;
            return speed0 * requiredTime;
        }
        break;
    case Phase::AccelerationFL100:
        /* get the maximum speed */
        if (this->m_cruiseSpeed < config.mtcdDepartureSpeedBelowFL100)
            maxSpeed = this->m_cruiseSpeed;
        else
            maxSpeed = config.mtcdDepartureSpeedBelowFL100;

        if (maxSpeed > speed0) {
            requiredTime = (maxSpeed - speed0) / this->m_acceleration;
            altitude1 = altitude0 + this->m_climbRateAcceleration * requiredTime;
            speed1 = maxSpeed;
            return speed0 * requiredTime + 0.5f * this->m_acceleration * requiredTime * requiredTime;
        }
        break;
    case Phase::ClimbFL100:
        /* get the maximum altitude */
        if (this->m_flight.flightPlan().flightLevel() < 10000.0_ft)
            maxAltitude = this->m_flight.flightPlan().flightLevel();
        else
            maxAltitude = 10000.0_ft;

        if (maxAltitude > altitude0) {
            requiredTime = (maxAltitude - altitude0) / this->m_climbRate;
            altitude1 = maxAltitude;
            speed1 = speed0;
            return speed0 * requiredTime;
        }
        break;
    case Phase::AccelerationCruise:
        if (this->m_cruiseSpeed > speed0) {
            requiredTime = (this->m_cruiseSpeed - speed0) / this->m_acceleration;
            altitude1 = altitude0 + this->m_climbRateAcceleration * requiredTime;
            speed1 = this->m_cruiseSpeed;
            return speed0 * requiredTime + 0.5f * this->m_acceleration * requiredTime * requiredTime;
        }
        break;
    case Phase::ClimbCruise:
        if (this->m_flight.flightPlan().flightLevel() > altitude0) {
            requiredTime = (this->m_flight.flightPlan().flightLevel() - altitude0) / this->m_climbRate;
            altitude1 = this->m_flight.flightPlan().flightLevel();
            speed1 = speed0;
            return speed0 * requiredTime;
        }
        break;
    default:
        break;
    }

    altitude1 = altitude0;
    requiredTime = 0.0_s;
    speed1 = speed0;
    return 0.0_m;
}

DepartureModel::Waypoint DepartureModel::predictWaypoint(const Waypoint& waypoint, const types::Coordinate& destination) const {
    types::Position position = waypoint.position;
    types::Velocity speed = waypoint.speed;

    while (true) {
        auto distance = position.coordinate().distanceTo(destination);
        types::Time reqTime, calcTime = 0.0_s;
        types::Length nextAltitude;
        types::Velocity nextSpeed;

        Phase phase = this->identifyPhase(position.altitude(), speed, this->m_flight.verticalSpeed());
        auto reqDistance = this->predictNextPhase(speed, position.altitude(), phase, nextSpeed, nextAltitude, reqTime);

        /* we need more space than available between two coordinates */
        if (reqDistance >= distance) {
            auto ratio = distance / reqDistance;

            /* calculate the waypoint */
            Waypoint wp;
            wp.position.setCoordinate(destination);
            wp.position.setAltitude(position.altitude() + (nextAltitude - position.altitude()) * ratio);
            wp.position.setHeading(position.coordinate().bearingTo(destination));
            wp.speed = speed + (nextSpeed - speed) * ratio;
            wp.reachingIn = waypoint.reachingIn + calcTime + ratio * reqTime;

            return wp;
        }
        /* we need less space than available */
        else {
            /* reached the last phase */
            if (Phase::ClimbCruise == phase) {
                Waypoint wp;

                wp.position.setCoordinate(destination);
                wp.position.setAltitude(nextAltitude);
                wp.position.setHeading(waypoint.position.coordinate().bearingTo(destination));
                wp.speed = nextSpeed;
                wp.reachingIn = waypoint.reachingIn + calcTime + position.coordinate().distanceTo(destination) / nextSpeed;

                return wp;
            }
            else {
                phase = static_cast<Phase>(static_cast<int>(phase) + 1);

                position.setHeading(position.coordinate().bearingTo(destination));
                position.setCoordinate(position.coordinate().projection(position.heading(), reqDistance));
                position.setAltitude(nextAltitude);

                speed = nextSpeed;
                calcTime += reqTime;
            }
        }
    }

    return Waypoint();
}

void DepartureModel::predictWaypoints(const std::vector<types::Coordinate>& waypoints) {
    this->m_waypoints.clear();
    this->m_waypoints.reserve(waypoints.size() + 1);
    this->m_routeCartesian.clear();

    /* transform the route to Cartesian coordinates to perform some intersection-tests */
    GeographicLib::Gnomonic projection(GeographicLib::Geodesic::WGS84());
    for (const auto& waypoint : std::as_const(waypoints)) {
        float x, y;

        projection.Forward(this->m_reference.latitude().convert(types::degree),
                           this->m_reference.longitude().convert(types::degree),
                           waypoint.latitude().convert(types::degree),
                           waypoint.longitude().convert(types::degree),
                           x, y);

        bg::append(this->m_routeCartesian, bg::model::point<float, 2, bg::cs::cartesian>(x, y));
    }

    std::vector<types::Coordinate>::const_iterator it;
    Waypoint prevPoint;

    prevPoint.position = this->m_flight.currentPosition();
    prevPoint.speed = this->m_flight.groundSpeed();
    prevPoint.reachingIn = 0.0_s;
    this->m_waypoints.push_back(prevPoint);
    if (this->m_flight.groundSpeed() < 5_kn)
        prevPoint.reachingIn = 20.0_s;

    for (it = waypoints.cbegin(); it < waypoints.cend(); ++it) {
        auto waypoint = this->predictWaypoint(prevPoint, *it);
        this->m_waypoints.push_back(waypoint);
        prevPoint = waypoint;
    }

    /* handle the last positions */
    while (waypoints.cend() != it) {
        auto distance = prevPoint.position.coordinate().distanceTo(*it);

        /* calculate the waypoint */
        Waypoint wp;
        wp.position.setCoordinate(*it);
        wp.position.setHeading(prevPoint.position.coordinate().bearingTo(*it));
        wp.speed = prevPoint.speed;
        wp.reachingIn = prevPoint.reachingIn + distance / prevPoint.speed;

        this->m_waypoints.push_back(wp);
    }
}

void DepartureModel::update(const types::Flight& flight, const std::vector<types::Coordinate>& waypoints) {
    /* get the DT and update the internal timestamp */
    auto now = std::chrono::system_clock::now();
    auto dt = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(now - this->m_lastUpdate).count()) * types::millisecond;
    this->m_lastUpdate = now;

    /* calculate the acceleration */
    auto acceleration = (flight.groundSpeed() - this->m_flight.groundSpeed()) / dt;
    auto climbRate = (flight.currentPosition().altitude() - this->m_flight.currentPosition().altitude()) / dt;

    /* update the internal data and identify the departure phase */
    this->m_flight = flight;
    this->m_currentPhase = this->identifyPhase(this->m_flight.currentPosition().altitude(), this->m_flight.groundSpeed(),
                                               this->m_flight.verticalSpeed());

    /* update some internal statistics to predict the positions better */
    switch (this->m_currentPhase) {
    case Phase::AccelerationFL100:
    case Phase::AccelerationCruise:
        this->m_acceleration = 0.9f * this->m_acceleration + 0.1f * acceleration;
        this->m_climbRateAcceleration = 0.9f * this->m_climbRateAcceleration + 0.1f * climbRate;
        break;
    case Phase::ClimbFL100:
    case Phase::ClimbCruise:
        this->m_climbRate = 0.9f * this->m_climbRate + 0.1f * climbRate;
        break;
    default:
        break;
    }

    this->predictWaypoints(waypoints);
}

bool DepartureModel::findSegment(const std::vector<Waypoint>& route, const types::Coordinate& point,
                                 std::size_t& startIdx, std::size_t& endIdx) {
    for (std::size_t i = 0; i < route.size() - 1; ++i) {
        if (route[i + 1].position.coordinate() == point) {
            startIdx = i;
            endIdx = i + 1;
            return true;
        }

        auto headingStart = route[i].position.coordinate().bearingTo(point);
        auto headingEnd = point.bearingTo(route[i + 1].position.coordinate());

        /* get the heading difference and normalize it */
        auto delta = headingStart - headingEnd;
        while (-1.0f * 180.0_deg > delta)
            delta += 360.0_deg;
        while (180.0_deg < delta)
            delta -= 360.0_deg;

        /* found the correct line segment */
        if (10_deg > delta.abs()) {
            startIdx = i;
            endIdx = i + 1;
            return true;
        }
    }

    return false;
}

types::Length DepartureModel::estimateHorizontalSpacing(const Waypoint& waypoint0, const Waypoint& waypoint1) {
    /*
     * estimate the horizontal spacing
     * idea:
     *  - calculate the time difference when both flights reach the point
     *  - use the closer one to calculate the traveled distance while the other flight travels to the point
     *  - this is in the end the horizontal spacing for the flight
     */
    types::Time dt;
    if (waypoint0.reachingIn >= waypoint1.reachingIn)
        dt = waypoint0.reachingIn - waypoint1.reachingIn;
    else
        dt = waypoint1.reachingIn - waypoint0.reachingIn;
    if (waypoint0.reachingIn <= waypoint1.reachingIn)
        return waypoint0.speed * dt;
    else
        return waypoint1.speed * dt;
}

std::list<DepartureModel::ConflictPosition> DepartureModel::findConflictCandidates(const DepartureModel& other) const {
    std::list<DepartureModel::ConflictPosition> retval;

    /* find all intersections */
    std::deque<bg::model::point<float, 2, bg::cs::cartesian>> intersections;
    bg::intersection(this->m_routeCartesian, other.m_routeCartesian, intersections);

    /* test all intersections */
    GeographicLib::Gnomonic projection(GeographicLib::Geodesic::WGS84());
    for (const auto& point : std::as_const(intersections)) {
        ConflictPosition conflict;
        float lat, lon;

        projection.Reverse(this->m_reference.latitude().convert(types::degree), this->m_reference.longitude().convert(types::degree),
                           point.get<0>(), point.get<1>(), lat, lon);

        conflict.coordinate = types::Coordinate(lon * types::degree, lat * types::degree);

        std::size_t startThis, startOther, endThis, endOther;
        bool foundSegments = DepartureModel::findSegment(this->m_waypoints, conflict.coordinate, startThis, endThis);
        foundSegments &= DepartureModel::findSegment(other.m_waypoints, conflict.coordinate, startOther, endOther);

        /* found relevant segments */
        if (true == foundSegments) {
            auto waypointThis = this->predictWaypoint(this->m_waypoints[startThis], conflict.coordinate);
            auto waypointOther = this->predictWaypoint(other.m_waypoints[startOther], conflict.coordinate);

            conflict.conflictIn = waypointThis.reachingIn;
            conflict.altitudeDifference = (waypointThis.position.altitude() - waypointOther.position.altitude()).abs();
            conflict.horizontalSpacing = DepartureModel::estimateHorizontalSpacing(waypointThis, waypointOther);

            retval.push_back(conflict);
        }
    }

    return retval;
}

const types::Flight& DepartureModel::flight() const {
    return this->m_flight;
}

const std::vector<DepartureModel::Waypoint>& DepartureModel::waypoints() const {
    return this->m_waypoints;
}
