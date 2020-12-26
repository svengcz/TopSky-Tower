/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the flight structure
 */

#include <types/Flight.h>

using namespace topskytower::types;

Flight::Flight() :
        m_callsign(),
        m_type(Flight::Type::Unknown),
        m_currentPosition(),
        m_groundSpeed(),
        m_verticalSpeed() { }

Flight::Flight(const std::string& callsign) :
        m_callsign(callsign),
        m_type(Flight::Type::Unknown),
        m_currentPosition(),
        m_groundSpeed(),
        m_verticalSpeed() { }

Flight::Type Flight::type() const {
    return this->m_type;
}

const std::string& Flight::callsign() const {
    return this->m_callsign;
}

const Position& Flight::currentPosition() const {
    return this->m_currentPosition;
}

const Velocity& Flight::groundSpeed() const {
    return this->m_groundSpeed;
}

const Velocity& Flight::verticalSpeed() const {
    return this->m_verticalSpeed;
}

void Flight::setType(Flight::Type type) {
    this->m_type = type;
}

void Flight::setCurrentPosition(const Position& position) {
    this->m_currentPosition = position;
}

void Flight::setGroundSpeed(const Velocity& groundSpeed) {
    this->m_groundSpeed = groundSpeed;
}

void Flight::setVerticalSpeed(const Velocity& verticalSpeed) {
    this->m_verticalSpeed = verticalSpeed;
}

Position Flight::predict(const Time& duration, const Velocity& minGroundSpeed) const {
    auto distance = (minGroundSpeed > this->m_groundSpeed ? minGroundSpeed : this->m_groundSpeed) * duration;
    auto altChange = this->m_verticalSpeed * duration;

    auto predCoordinate = this->m_currentPosition.coordinate().projection(this->m_currentPosition.heading(), distance);
    auto predAltitude = this->m_currentPosition.altitude() + altChange;
    if (0.0_m > predAltitude)
        predAltitude = 0.0_m;

    return Position(predCoordinate, predAltitude, this->m_currentPosition.heading());
}
