/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the position to abstract global coordinates
 */

#include <types/Position.h>

using namespace topskytower::types;

Position::Position() :
        m_coordinate(),
        m_altitude() { }

Position::Position(const Coordinate& coordinate, const Length& altitude) :
        m_coordinate(coordinate),
        m_altitude(altitude) { }

const Coordinate& Position::coordinate() const {
    return this->m_coordinate;
}

const Length& Position::altitude() const {
    return this->m_altitude;
}
