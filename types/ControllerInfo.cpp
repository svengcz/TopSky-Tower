/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the controller information
 */

#include <types/ControllerInfo.h>

using namespace topskytower::types;

ControllerInfo::ControllerInfo() :
        m_identifier(),
        m_prefix(),
        m_midfix(),
        m_suffix(),
        m_primaryFrequency() { }

ControllerInfo::ControllerInfo(const std::string& identifer, const std::string& prefix, const std::string& midfix,
                               const std::string& suffix, const std::string& primaryFrequency) :
        m_identifier(identifer),
        m_prefix(prefix),
        m_midfix(midfix),
        m_suffix(suffix),
        m_primaryFrequency(primaryFrequency) { }

const std::string& ControllerInfo::identifier() const {
    return this->m_identifier;
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
