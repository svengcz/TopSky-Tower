/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the waypoint
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <types/Waypoint.h>

using namespace topskytower;
using namespace topskytower::types;

Waypoint::Waypoint() :
        m_name(),
        m_position() { }

Waypoint::Waypoint(const std::string& name, const types::Coordinate& position) :
        m_name(name),
        m_position(position) { }

const std::string& Waypoint::name() const {
    return this->m_name;
}

const types::Coordinate& Waypoint::position() const {
    return this->m_position;
}
