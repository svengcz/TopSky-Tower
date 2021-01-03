/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the euroscope file format
 */

#include <filesystem>

#include <helper/Exception.h>
#include <helper/String.h>
#include <formats/AirportFileFormat.h>

#include "IniFileFormat.h"

using namespace topskytower;
using namespace topskytower::formats;

void AirportFileFormat::parseSid(const std::string& line, types::StandardInstrumentDeparture& sid) {
    if (std::string::npos == line.find("SID:"))
        return;

    auto split = helper::String::splitString(line, ":");
    if (4 != split.size())
        return;

    sid.name = split[1];
    sid.clearanceLimit = static_cast<float>(std::atoi(split[2].c_str())) * types::feet;
    sid.containsStepClimbs = '0' != split[3][0];
}

AirportFileFormat::AirportFileFormat(const std::string& filename) :
        m_configurations() {
    IniFileFormat file(filename);

    for (const auto& block : std::as_const(file.m_blocks)) {
        std::string icao(block.first);
        helper::String::stringReplace(icao, "[", "");
        helper::String::stringReplace(icao, "]", "");

        this->m_configurations[icao].valid = true;
        for (const auto& line : std::as_const(block.second)) {
            types::StandardInstrumentDeparture sid;

            this->parseSid(line, sid);
            this->m_configurations[icao].sids[sid.name] = sid;
        }
    }
}

const types::AirportConfiguration& AirportFileFormat::configuration(const std::string& icao) const {
    static types::AirportConfiguration fallback;
    auto it = this->m_configurations.find(icao);
    if (this->m_configurations.cend() != it)
        return it->second;
    else
        return fallback;
}
