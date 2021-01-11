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
        m_atcCommand(),
        m_origin(),
        m_departureRoute(),
        m_departureRunway(),
        m_destination(),
        m_arrivalRunway(),
        m_flightLevel(),
        m_arrivalRoute(),
        m_assignedSquawk(0),
        m_clearanceLimit(),
        m_clearanceFlags(false),
        m_rnavCapable(false),
        m_transponderExists(false),
        m_route() { }

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

void FlightPlan::setFlightLevel(const types::Length& flightLevel) {
    this->m_flightLevel = flightLevel;
}

const types::Length& FlightPlan::flightLevel() const {
    return this->m_flightLevel;
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

void FlightPlan::setRoute(Route&& route) {
    this->m_route = std::move(route);
}

const Route& FlightPlan::route() const {
    return this->m_route;
}

void FlightPlan::setDepartureRunway(const std::string& runway) {
    this->m_departureRunway = runway;
}

const std::string& FlightPlan::departureRunway() const {
    return this->m_departureRunway;
}

void FlightPlan::setArrivalRunway(const std::string& runway) {
    this->m_arrivalRunway = runway;
}

const std::string& FlightPlan::arrivalRunway() const {
    return this->m_arrivalRunway;
}

void FlightPlan::setFlag(FlightPlan::AtcCommand command) {
    std::uint8_t mask = static_cast<std::uint8_t>(command);

    /* arrival-command given */
    if (0 != (mask & 0xf0)) {
        this->m_atcCommand &= 0x0f;
        this->m_atcCommand |= mask;
    }
    /* departure-command given */
    else {
        this->m_atcCommand &= 0xf0;
        this->m_atcCommand |= mask;
    }
}

FlightPlan::AtcCommand FlightPlan::departureFlag() const {
    return static_cast<FlightPlan::AtcCommand>(this->m_atcCommand & 0x0f);
}

FlightPlan::AtcCommand FlightPlan::arrivalFlag() const {
    return static_cast<FlightPlan::AtcCommand>(this->m_atcCommand & 0xf0);
}
