/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the airport file format
 */

#include <filesystem>

#include <helper/Exception.h>
#include <helper/String.h>
#include <formats/AirportFileFormat.h>

#include "IniFileFormat.h"

using namespace topskytower;
using namespace topskytower::formats;
using namespace topskytower::types;

bool AirportFileFormat::parseSid(const std::string& line, types::StandardInstrumentDeparture& sid) {
    auto split = helper::String::splitString(line, ":");
    if (9 != split.size())
        return false;
    if ("SID" != split[0])
        return false;
    if (0 == split[1].length())
        return false;
    if (0 == split[2].length())
        return false;

    /* translate the mandatory entries */
    sid.name = split[1];
    sid.clearanceLimit = static_cast<float>(std::atoi(split[2].c_str())) * types::feet;

    /* translate the optional entries */
    if (0 != split[3].length())
        sid.containsStepClimbs = '0' != split[3][0];
    if (0 != split[4].length()) {
        switch (split[4][0]) {
        case 'J':
            sid.engineType = types::Aircraft::EngineType::Jet;
            break;
        case 'T':
            sid.engineType = types::Aircraft::EngineType::Turboprop;
            break;
        case 'E':
            sid.engineType = types::Aircraft::EngineType::Electric;
            break;
        default:
            return false;
        }
    }
    if (0 != split[5].length())
        sid.requiresTransponder = '0' != split[5][0];
    if (0 != split[6].length())
        sid.requiresRnav = '0' != split[6][0];
    if (0 != split[7].length())
        sid.minimumCruiseLevel = static_cast<float>(std::atoi(split[7].c_str())) * types::feet;
    if (0 != split[8].length())
        sid.maximumCruiseLevel = static_cast<float>(std::atoi(split[8].c_str())) * types::feet;
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
            types::StandardInstrumentDeparture sid = {
                "",
                0_ft,
                false,
                types::Aircraft::EngineType::Unknown,
                false,
                false,
                0_ft,
                99000_ft
            };

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
