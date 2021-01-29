/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the position to abstract global coordinates
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <types/Position.h>

using namespace topskytower::types;

Position::Position() :
        m_coordinate(),
        m_altitude(),
        m_heading() { }

Position::Position(const Coordinate& coordinate, const Length& altitude, const Angle& heading) :
        m_coordinate(coordinate),
        m_altitude(altitude),
        m_heading(heading) { }

void Position::setCoordinate(const Coordinate& coordinate) {
    this->m_coordinate = coordinate;
}

const Coordinate& Position::coordinate() const {
    return this->m_coordinate;
}

void Position::setAltitude(const Length& altitude) {
    this->m_altitude = altitude;
}

const Length& Position::altitude() const {
    return this->m_altitude;
}

void Position::setHeading(const Angle& heading) {
    this->m_heading = heading;
}

const Angle& Position::heading() const {
    return this->m_heading;
}
