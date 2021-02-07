/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the aircraft file format
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <fstream>

#include <helper/Exception.h>
#include <helper/String.h>
#include <formats/AircraftFileFormat.h>

using namespace topskytower;
using namespace topskytower::formats;

bool AircraftFileFormat::parseAircraft(const std::string& line) {
    auto split = helper::String::splitString(line, ":");
    if (6 != split.size())
        return false;

    types::Aircraft aircraft;
    aircraft.setIcaoCode(split[0]);
    aircraft.setWingspan(static_cast<float>(std::atof(split[1].c_str())) * types::metre);
    aircraft.setLength(static_cast<float>(std::atof(split[2].c_str())) * types::metre);
    aircraft.setHeight(static_cast<float>(std::atof(split[3].c_str())) * types::metre);
    aircraft.setMTOW(static_cast<float>(std::atof(split[4].c_str())) * types::kilogram);

    this->m_aircrafts[aircraft.icaoCode()] = aircraft;

    return true;
}

AircraftFileFormat::AircraftFileFormat(const std::string& filename) :
        FileFormat(),
        m_aircrafts() {
    std::ifstream stream(filename);

    std::uint32_t lineNumber = 1;
    for (std::string line; std::getline(stream, line); ++lineNumber) {
        if (false == this->parseAircraft(line)) {
            this->m_errorMessage = "Invalid aircraft entry";
            this->m_errorLine = lineNumber;
            return;
        }
    }
}

const std::map<std::string, types::Aircraft>& AircraftFileFormat::aircrafts() const {
    return this->m_aircrafts;
}
