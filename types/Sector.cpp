/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the sector definition
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <helper/Exception.h>
#include <types/Sector.h>

using namespace topskytower;
using namespace topskytower::types;

Sector::Sector() :
        m_info(),
        m_type(Type::Undefined),
        m_borders() { }

void Sector::parseSectorType() {
    if ("DEL" == this->m_info.suffix())
        this->m_type = Type::Delivery;
    else if ("GND" == this->m_info.suffix())
        this->m_type = Type::Ground;
    else if ("TWR" == this->m_info.suffix())
        this->m_type = Type::Tower;
    else if ("APP" == this->m_info.suffix())
        this->m_type = Type::Approach;
    else if ("DEP" == this->m_info.suffix())
        this->m_type = Type::Departure;
    else if ("CTR" == this->m_info.suffix())
        this->m_type = Type::Center;
    else if ("ATIS" == this->m_info.suffix())
        this->m_type = Type::ATIS;
    else if ("FSS" == this->m_info.suffix())
        this->m_type = Type::FlightService;
    else
        throw helper::Exception("Sector", "Unknown station suffix (" + this->m_info.suffix() + ") for " + this->m_info.identifier());
}

Sector::Sector(std::string&& identifier, std::string&& prefix, std::string&& midfix,
               std::string&& suffix, std::string&& frequency) :
        m_info(identifier, prefix, midfix, suffix, frequency, ""),
        m_type(Type::Undefined),
        m_borders() {
    this->parseSectorType();
}

Sector::Sector(std::string&& identifier, std::string&& prefix, std::string&& midfix,
               std::string&& suffix, std::string&& frequency, const std::string& latitude,
               const std::string& longitude) :
        m_info(identifier, prefix, midfix, suffix, frequency, "", latitude, longitude),
        m_type(Type::Undefined),
        m_borders() {
    this->parseSectorType();
}

Sector::Sector(const Sector& other) noexcept :
    m_info(other.m_info),
    m_type(other.m_type),
    m_borders(other.m_borders) { }

Sector::Sector(Sector&& other) noexcept :
        m_info(std::move(other.m_info)),
        m_type(other.m_type),
        m_borders(std::move(other.m_borders)) { }

Sector& Sector::operator=(const Sector& other) noexcept {
    if (&other != this) {
        this->m_info = other.m_info;
        this->m_type = other.m_type;
        this->m_borders = other.m_borders;
    }
    return *this;
}

Sector& Sector::operator=(Sector&& other) noexcept {
    if (&other != this) {
        this->m_info = std::move(other.m_info);
        this->m_type = other.m_type;
        this->m_borders = std::move(other.m_borders);
    }
    return *this;
}

const ControllerInfo& Sector::controllerInfo() const {
    return this->m_info;
}

Sector::Type Sector::type() const {
    return this->m_type;
}

void Sector::setBorders(std::list<SectorBorder>&& borders) {
    this->m_borders = std::move(borders);
    this->m_borders.sort([](const SectorBorder& b0, const SectorBorder& b1) {
        return b0.upperAltitude() < b1.upperAltitude();
    });
}

const std::list<SectorBorder>& Sector::borders() const {
    return this->m_borders;
}

bool Sector::isInsideSector(const types::Coordinate& coordinate) const {
    for (const auto& border : std::as_const(this->m_borders)) {
        if (true == border.isInsideBorder(coordinate))
            return true;
    }

    return false;
}

bool Sector::isInsideSector(const types::Position& position) const {
    for (const auto& border : std::as_const(this->m_borders)) {
        if (true == border.isInsideBorder(position))
            return true;
    }

    return false;
}
