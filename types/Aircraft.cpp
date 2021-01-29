/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the aircraft structure
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <types/Aircraft.h>

using namespace topskytower;
using namespace topskytower::types;

Aircraft::Aircraft() :
        m_code(),
        m_engineType(types::Aircraft::EngineType::Unknown),
        m_engineCount(0),
        m_wtc(types::Aircraft::WTC::Unknown),
        m_length(),
        m_wingspan(),
        m_height(),
        m_maxTakeoffWeight() { }

void Aircraft::setIcaoCode(const std::string& code) {
    this->m_code = code;
}

const std::string& Aircraft::icaoCode() const {
    return this->m_code;
}

void Aircraft::setEngineType(Aircraft::EngineType type) {
    this->m_engineType = type;
}

Aircraft::EngineType Aircraft::engineType() const {
    return this->m_engineType;
}

void Aircraft::setEngineCount(std::uint8_t count) {
    this->m_engineCount = count;
}

std::uint8_t Aircraft::engineCount() const {
    return this->m_engineCount;
}

void Aircraft::setWTC(Aircraft::WTC wtc) {
    this->m_wtc = wtc;
}

Aircraft::WTC Aircraft::wtc() const {
    return this->m_wtc;
}

void Aircraft::setLength(const types::Length& length) {
    this->m_length = length;
}

const types::Length& Aircraft::length() const {
    return this->m_length;
}

void Aircraft::setWingspan(const types::Length& wingspan) {
    this->m_wingspan = wingspan;
}

const types::Length& Aircraft::wingspan() const {
    return this->m_wingspan;
}

void Aircraft::setHeight(const types::Length& height) {
    this->m_height = height;
}

const types::Length& Aircraft::height() const {
    return this->m_height;
}

void Aircraft::setMTOW(const types::Mass& mtow) {
    this->m_maxTakeoffWeight = mtow;
}

const types::Mass& Aircraft::mtow() const {
    return this->m_maxTakeoffWeight;
}
