/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the sector definition
 */

#include <helper/Exception.h>
#include <types/Sector.h>

using namespace topskytower;
using namespace topskytower::types;

Sector::Sector() :
        m_identifier(),
        m_type(Type::Undefined),
        m_prefix(),
        m_midfix(),
        m_suffix(),
        m_frequency(),
        m_borders() { }

Sector::Sector(std::string&& identifier, std::string&& prefix, std::string&& midfix,
               std::string&& suffix, std::string&& frequency) :
        m_identifier(std::move(identifier)),
        m_type(Type::Undefined),
        m_prefix(std::move(prefix)),
        m_midfix(std::move(midfix)),
        m_suffix(std::move(suffix)),
        m_frequency(std::move(frequency)),
        m_borders() {
    if ("DEL" == this->m_suffix)
        this->m_type = Type::Delivery;
    else if ("GND" == this->m_suffix)
        this->m_type = Type::Ground;
    else if ("TWR" == this->m_suffix)
        this->m_type = Type::Tower;
    else if ("APP" == this->m_suffix)
        this->m_type = Type::Approach;
    else if ("DEP" == this->m_suffix)
        this->m_type = Type::Departure;
    else if ("CTR" == this->m_suffix)
        this->m_type = Type::Center;
    else if ("ATIS" == this->m_suffix)
        this->m_type = Type::ATIS;
    else if ("FSS" == this->m_suffix)
        this->m_type = Type::FlightService;
    else
        throw helper::Exception("Sector", "Unknown station suffix (" + this->m_suffix + ") for " + this->m_identifier);
}

Sector::Sector(const Sector& other) noexcept :
    m_identifier(other.m_identifier),
    m_type(other.m_type),
    m_prefix(other.m_prefix),
    m_midfix(other.m_midfix),
    m_suffix(other.m_suffix),
    m_frequency(other.m_frequency),
    m_borders(other.m_borders) { }

Sector::Sector(Sector&& other) noexcept :
        m_identifier(std::move(other.m_identifier)),
        m_type(other.m_type),
        m_prefix(std::move(other.m_prefix)),
        m_midfix(std::move(other.m_midfix)),
        m_suffix(std::move(other.m_suffix)),
        m_frequency(std::move(other.m_frequency)),
        m_borders(std::move(other.m_borders)) { }

Sector& Sector::operator=(const Sector& other) noexcept {
    if (&other != this) {
        this->m_identifier = other.m_identifier;
        this->m_type = other.m_type;
        this->m_prefix = other.m_prefix;
        this->m_midfix = other.m_midfix;
        this->m_suffix = other.m_suffix;
        this->m_frequency = other.m_frequency;
        this->m_borders = other.m_borders;
    }
    return *this;
}

Sector& Sector::operator=(Sector&& other) noexcept {
    if (&other != this) {
        this->m_identifier = std::move(other.m_identifier);
        this->m_type = other.m_type;
        this->m_prefix = std::move(other.m_prefix);
        this->m_midfix = std::move(other.m_midfix);
        this->m_suffix = std::move(other.m_suffix);
        this->m_frequency = std::move(other.m_frequency);
        this->m_borders = std::move(other.m_borders);
    }
    return *this;
}

const std::string& Sector::identifier() const {
    return this->m_identifier;
}

Sector::Type Sector::type() const {
    return this->m_type;
}

const std::string& Sector::prefix() const {
    return this->m_prefix;
}

const std::string& Sector::midfix() const {
    return this->m_midfix;
}

const std::string& Sector::suffix() const {
    return this->m_suffix;
}

const std::string& Sector::frequency() const {
    return this->m_frequency;
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
