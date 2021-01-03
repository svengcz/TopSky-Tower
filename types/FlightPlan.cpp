/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the flight plan
 */

#include <types/FlightPlan.h>

using namespace topskytower;
using namespace topskytower::types;

FlightPlan::FlightPlan() :
        m_type(FlightPlan::Type::Unknown),
        m_aircraft(),
        m_origin(),
        m_departureRoute(),
        m_destination(),
        m_arrivalRoute(),
        m_assignedSquawk(0),
        m_clearanceLimit(),
        m_clearanceFlags(false),
        m_rnavCapable(false),
        m_transponderExists(false) { }

void FlightPlan::setType(FlightPlan::Type type) {
    this->m_type = type;
}

FlightPlan::Type FlightPlan::type() const {
    return this->m_type;
}

void FlightPlan::setAircraft(const types::Aircraft& aircraft) {
    this->m_aircraft = aircraft;
}

const types::Aircraft& FlightPlan::aircraft() const {
    return this->m_aircraft;
}

void FlightPlan::setOrigin(const std::string& origin) {
    this->m_origin = origin;
}

const std::string& FlightPlan::origin() const {
    return this->m_origin;
}

void FlightPlan::setDestination(const std::string& destination) {
    this->m_destination = destination;
}

const std::string& FlightPlan::destination() const {
    return this->m_destination;
}

void FlightPlan::setDepartureRoute(const std::string& route) {
    this->m_departureRoute = route;
}

const std::string& FlightPlan::departureRoute() const {
    return this->m_departureRoute;
}

void FlightPlan::setArrivalRoute(const std::string& route) {
    this->m_arrivalRoute = route;
}

const std::string& FlightPlan::arrivalRoute() const {
    return this->m_arrivalRoute;
}

void FlightPlan::setClearanceLimit(const Length& altitude) {
    this->m_clearanceLimit = altitude;
}

const Length& FlightPlan::clearanceLimit() const {
    return this->m_clearanceLimit;
}

void FlightPlan::setClearanceFlag(bool flag) {
    this->m_clearanceFlags = flag;
}

bool FlightPlan::clearanceFlag() const {
    return this->m_clearanceFlags;
}

void FlightPlan::setAssignedSquawk(std::uint16_t squawk) {
    this->m_assignedSquawk = squawk;
}

std::uint16_t FlightPlan::assignedSquawk() const {
    return this->m_assignedSquawk;
}

void FlightPlan::setRnavCapable(bool capable) {
    this->m_rnavCapable = capable;
}

bool FlightPlan::rnavCapable() const {
    return this->m_rnavCapable;
}

void FlightPlan::setTransponderExistence(bool exists) {
    this->m_transponderExists = exists;
}

bool FlightPlan::transponderExists() const {
    return this->m_transponderExists;
}
