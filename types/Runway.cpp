/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the runway
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <types/runway.h>

using namespace topskytower::types;

Runway::Runway() :
        m_name(),
        m_start(),
        m_end(),
        m_heading(),
        m_length() { }

Runway::Runway(const std::string& name, const Coordinate& start, const Coordinate& end) :
        m_name(name),
        m_start(start),
        m_end(end),
        m_heading(start.bearingTo(end)),
        m_length(start.distanceTo(end)) { }

const std::string& Runway::name() const {
    return this->m_name;
}

const Coordinate& Runway::start() const {
    return this->m_start;
}

const Coordinate& Runway::end() const {
    return this->m_end;
}

const Angle& Runway::heading() const {
    return this->m_heading;
}

const Length& Runway::length() const {
    return this->m_length;
}
