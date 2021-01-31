/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the controller information
 */

#include <helper/String.h>
#include <types/ControllerInfo.h>

using namespace topskytower;
using namespace topskytower::types;

ControllerInfo::ControllerInfo() :
        m_identifier(),
        m_prefix(),
        m_midfix(),
        m_suffix(),
        m_callsign(),
        m_primaryFrequency(),
        m_controllerName(),
        m_center() { }

ControllerInfo::ControllerInfo(const std::string& identifier, const std::string& callsign,
                               const std::string& primaryFrequency, const std::string& fullName) :
        m_identifier(identifier),
        m_prefix(),
        m_midfix(),
        m_suffix(),
        m_callsign(callsign),
        m_primaryFrequency(primaryFrequency),
        m_controllerName(fullName),
        m_center() {
    auto split = helper::String::splitString(callsign, "_");
    if (3 == split.size() && 0 != split[1].length()) {
        this->m_prefix = split[0];
        this->m_midfix = split[1];
        this->m_suffix = split[2];
    }
    else {
        this->m_prefix = split.front();
        this->m_suffix = split.back();
    }
}

ControllerInfo::ControllerInfo(const std::string& identifier, const std::string& prefix, const std::string& midfix,
                               const std::string& suffix, const std::string& primaryFrequency, const std::string& fullName,
                               const std::string& latitude, const std::string& longitude) :
        m_identifier(identifier),
        m_prefix(prefix),
        m_midfix(midfix),
        m_suffix(suffix),
        m_primaryFrequency(primaryFrequency),
        m_controllerName(fullName),
        m_center(longitude, latitude) { }

bool ControllerInfo::operator==(const ControllerInfo& other) const {
    return this->m_prefix == other.m_prefix && this->m_midfix == other.m_midfix && this->m_suffix == other.m_suffix;
}

bool ControllerInfo::operator!=(const ControllerInfo& other) const {
    return false == this->operator==(other);
}

const std::string& ControllerInfo::identifier() const {
    return this->m_identifier;
}

const std::string& ControllerInfo::callsign() const {
    return this->m_callsign;
}

const std::string& ControllerInfo::prefix() const {
    return this->m_prefix;
}

const std::string& ControllerInfo::midfix() const {
    return this->m_midfix;
}

const std::string& ControllerInfo::suffix() const {
    return this->m_suffix;
}

const std::string& ControllerInfo::primaryFrequency() const {
    return this->m_primaryFrequency;
}

const std::string& ControllerInfo::controllerName() const {
    return this->m_controllerName;
}

const types::Coordinate& ControllerInfo::centerPoint() const {
    return this->m_center;
}
