/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the flight structure
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <types/Flight.h>

using namespace topskytower::types;

Flight::Flight() :
        m_flightPlan(),
        m_callsign(),
        m_airborne(false),
        m_currentPosition(),
        m_groundSpeed(),
        m_verticalSpeed(),
        m_markedByController(false),
        m_onMissedApproach(false),
        m_irregularFlight(false),
        m_establishedOnILS(false),
        m_departureReady(false),
        m_isTrackedByController(false),
        m_isTrackedByOtherController(false),
        m_handoffReceivedBy() { }

Flight::Flight(const std::string& callsign) :
        m_flightPlan(),
        m_airborne(false),
        m_callsign(callsign),
        m_currentPosition(),
        m_groundSpeed(),
        m_verticalSpeed(),
        m_markedByController(false),
        m_onMissedApproach(false),
        m_irregularFlight(false),
        m_establishedOnILS(false),
        m_departureReady(false),
        m_isTrackedByOtherController(false),
        m_handoffReceivedBy() { }

const std::string& Flight::callsign() const {
    return this->m_callsign;
}

bool Flight::airborne() const {
    return this->m_airborne;
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

bool Flight::markedByController() const {
    return this->m_markedByController;
}

bool Flight::onMissedApproach() const {
    return this->m_onMissedApproach;
}

bool Flight::irregularHandoff() const {
    return this->m_irregularFlight;
}

bool Flight::establishedOnILS() const {
    return this->m_establishedOnILS;
}

bool Flight::readyForDeparture() const {
    return this->m_departureReady;
}

const FlightPlan& Flight::flightPlan() const {
    return this->m_flightPlan;
}

FlightPlan& Flight::flightPlan() {
    return this->m_flightPlan;
}

void Flight::setAirborne(bool airborne) {
    this->m_airborne = airborne;
}

void Flight::setCurrentPosition(const Position& position) {
    this->m_currentPosition = position;
}

void Flight::setGroundSpeed(const Velocity& groundSpeed) {
    this->m_groundSpeed = groundSpeed;
}

void Flight::setReadyForDeparture(bool value) {
    this->m_departureReady = value;
}

void Flight::setVerticalSpeed(const Velocity& verticalSpeed) {
    this->m_verticalSpeed = verticalSpeed;
}

void Flight::setMarkedByController(bool value) {
    this->m_markedByController = value;
}

void Flight::setOnMissedApproach(bool value) {
    this->m_onMissedApproach = value;
}

void Flight::setIrregularHandoff(bool value) {
    this->m_irregularFlight = value;
}

void Flight::setEstablishedOnILS(bool value) {
    this->m_establishedOnILS = value;
}

void Flight::setFlightPlan(const FlightPlan& plan) {
    this->m_flightPlan = plan;
}

bool Flight::isTracked() const {
    return this->m_isTrackedByController;
}

void Flight::setTrackedState(bool state) {
    this->m_isTrackedByController = state;
}

bool Flight::isTrackedByOther() const {
    return this->m_isTrackedByOtherController;
}

void Flight::setTrackedByOtherState(bool state) {
    this->m_isTrackedByOtherController = state;
}

void Flight::setHandoffInitiatedId(const std::string& id) {
    this->m_handoffReceivedBy = id;
}

const std::string& Flight::handoffInitiatedId() const {
    return this->m_handoffReceivedBy;
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
