/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the runway
 */

#include <types/runway.h>

using namespace topskytower::types;

Runway::Runway() :
        m_name(),
        m_start(),
        m_end(),
        m_heading(),
        m_length() { }

Runway::Runway(const std::string& name, const Coordinate& start, const Coordinate& end,
               const types::Angle& heading, const types::Length& length) :
        m_name(name),
        m_start(start),
        m_end(end),
        m_heading(heading),
        m_length(length) { }

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
