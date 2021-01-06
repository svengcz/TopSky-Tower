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

bool AirportFileFormat::parseSid(const std::vector<std::string>& elements, types::StandardInstrumentDeparture& sid) {
    if (9 != elements.size())
        return false;
    if (0 == elements[1].length())
        return false;
    if (0 == elements[2].length())
        return false;

    /* translate the mandatory entries */
    sid.name = elements[1];
    sid.clearanceLimit = static_cast<float>(std::atoi(elements[2].c_str())) * types::feet;

    /* translate the optional entries */
    if (0 != elements[3].length())
        sid.containsStepClimbs = '0' != elements[3][0];
    if (0 != elements[4].length()) {
        switch (elements[4][0]) {
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
    if (0 != elements[5].length())
        sid.requiresTransponder = '0' != elements[5][0];
    if (0 != elements[6].length())
        sid.requiresRnav = '0' != elements[6][0];
    if (0 != elements[7].length())
        sid.minimumCruiseLevel = static_cast<float>(std::atoi(elements[7].c_str())) * types::feet;
    if (0 != elements[8].length())
        sid.maximumCruiseLevel = static_cast<float>(std::atoi(elements[8].c_str())) * types::feet;

    return true;
}

bool AirportFileFormat::parseConstraint(const std::vector<std::string>& elements, types::DestinationConstraint& constraint) {
    if (5 != elements.size())
        return false;
    if (0 == elements[1].length())
        return false;
    if (0 == elements[2].length())
        return false;

    constraint.destination = elements[1];
    constraint.evenCruiseLevel = '0' != elements[2][0];
    if (0 != elements[3].length())
        constraint.minimumCruiseLevel = static_cast<float>(std::atoi(elements[3].c_str())) * types::feet;
    if (0 != elements[4].length())
        constraint.maximumCruiseLevel = static_cast<float>(std::atoi(elements[4].c_str())) * types::feet;

    return true;
}

bool AirportFileFormat::parseDepartures(const std::vector<std::string>& lines) {
    for (const auto& line : std::as_const(lines)) {
        auto split = helper::String::splitString(line, ":");

        if ("SID" == split[0]) {
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

            if (true == AirportFileFormat::parseSid(split, sid))
                this->m_configuration.sids[sid.name] = sid;
        }
        else if ("CSTR" == split[0]) {
            types::DestinationConstraint constraint = {
                "",
                false,
                0_ft,
                99000_ft
            };

            if (true == AirportFileFormat::parseConstraint(split, constraint))
                this->m_configuration.destinationConstraints.push_back(constraint);
        }
    }

    return true;
}

bool AirportFileFormat::parseStandDefinition(const std::vector<std::string>& elements, types::Stand& stand) {
    stand.name = elements[2];
    stand.position = types::Coordinate(elements[4], elements[3]);
    stand.assignmentRadius = static_cast<float>(std::atoi(elements[5].c_str())) * types::metre;
    return true;
}

static __inline void __resetStand(types::Stand& stand) {
    stand.name = "";
    stand.blockingStands.clear();
    stand.priority = 0;
    stand.manualAssignment = false;
    stand.wingspan[0] = 0.0_m;
    stand.wingspan[1] = 1000.0_m;
    stand.length[0] = 0.0_m;
    stand.length[1] = 1000.0_m;
    stand.height[0] = 0.0_m;
    stand.height[1] = 1000.0_m;
    stand.wtcWhitelist.clear();
    stand.wtcBlacklist.clear();
    stand.engineTypeWhitelist.clear();
    stand.engineTypeBlacklist.clear();
}

static __inline bool __parseLengthRange(const std::vector<std::string>& elements, types::Length& minimum, types::Length& maximum) {
    if (2 == elements.size()) {
        minimum = 0.0_m;
        maximum = static_cast<float>(std::atof(elements[1].c_str())) * types::metre;
        return true;
    }
    else if (3 == elements.size()) {
        minimum = static_cast<float>(std::atof(elements[1].c_str())) * types::metre;
        maximum = static_cast<float>(std::atof(elements[2].c_str())) * types::metre;
        return true;
    }

    return false;
}

bool AirportFileFormat::parseWingspan(const std::vector<std::string>& elements, types::Stand& stand) {
    return __parseLengthRange(elements, stand.wingspan[0], stand.wingspan[1]);
}

bool AirportFileFormat::parseLength(const std::vector<std::string>& elements, types::Stand& stand) {
    return __parseLengthRange(elements, stand.length[0], stand.length[1]);
}

bool AirportFileFormat::parseHeight(const std::vector<std::string>& elements, types::Stand& stand) {
    return __parseLengthRange(elements, stand.length[0], stand.length[1]);
}

bool AirportFileFormat::parseWtc(const std::string& categories, std::list<types::Aircraft::WTC>& list) {
    for (const auto& category : std::as_const(categories)) {
        switch (category) {
        case 'L':
            list.push_back(types::Aircraft::WTC::Light);
            break;
        case 'M':
            list.push_back(types::Aircraft::WTC::Medium);
            break;
        case 'H':
            list.push_back(types::Aircraft::WTC::Heavy);
            break;
        case 'J':
            list.push_back(types::Aircraft::WTC::Super);
            break;
        default:
            return false;
        }
    }

    return true;
}

static __inline types::Aircraft::EngineType __translateEngineType(char type) {
    switch (type) {
    case 'P':
    case 'T':
        return types::Aircraft::EngineType::Turboprop;
    case 'E':
        return types::Aircraft::EngineType::Electric;
    case 'J':
    default:
        return types::Aircraft::EngineType::Jet;
    }
}

bool AirportFileFormat::parseEngineType(const std::string& types, std::list<types::Aircraft::EngineType>& list) {
    if (std::string::npos == types.find(",")) {
        for (const auto& type : std::as_const(types))
            list.push_back(__translateEngineType(type));
    }
    else {
        auto split = helper::String::splitString(types, ",");
        for (const auto& type : std::as_const(types))
            list.push_back(__translateEngineType(type));
    }

    return true;
}

bool AirportFileFormat::parseStands(const std::vector<std::string>& lines) {
    types::Stand stand;
    __resetStand(stand);

    for (const auto& line : std::as_const(lines)) {
        auto split = helper::String::splitString(line, ":");
        if (1 > split.size() && 0 == split[0].size())
            continue;

        if ("STAND" == split[0] && 6 == split.size()) {
            if (0 != stand.name.length())
                this->m_configuration.aircraftStands.push_back(stand);

            __resetStand(stand);
            AirportFileFormat::parseStandDefinition(split, stand);
        }
        else if ("WINGSPAN" == split[0] && 2 <= split.size()) {
            AirportFileFormat::parseWingspan(split, stand);
        }
        else if ("LENGTH" == split[0] && 2 <= split.size()) {
            AirportFileFormat::parseLength(split, stand);
        }
        else if ("HEIGHT" == split[0] && 2 <= split.size()) {
            AirportFileFormat::parseHeight(split, stand);
        }
        else if ("MANUAL" == split[0] && 1 == split.size()) {
            stand.manualAssignment = true;
        }
        else if ("BLOCKS" == split[0] && 2 == split.size()) {
            stand.blockingStands = helper::String::splitString(split[1], ",");
        }
        else if ("WTC" == split[0] && 2 == split.size()) {
            AirportFileFormat::parseWtc(split[1], stand.wtcWhitelist);
        }
        else if ("NOTWTC" == split[0] && 2 == split.size()) {
            AirportFileFormat::parseWtc(split[1], stand.wtcBlacklist);
        }
        else if ("ENGINETYPE" == split[0] && 2 == split.size()) {
            AirportFileFormat::parseEngineType(split[1], stand.engineTypeWhitelist);
        }
        else if ("NOTENGINETYPE" == split[0] && 2 == split.size()) {
            AirportFileFormat::parseEngineType(split[1], stand.engineTypeBlacklist);
        }
        else if ("PRIORITY" == split[0] && 2 == split.size()) {
            stand.priority = std::atoi(split[1].c_str());
        }
    }

    if (0 != stand.name.length())
        this->m_configuration.aircraftStands.push_back(stand);

    return true;
}

AirportFileFormat::AirportFileFormat(const std::string& filename) :
        m_configuration() {
    IniFileFormat file(filename);

    for (const auto& block : std::as_const(file.m_blocks)) {
        this->m_configuration.valid = true;

        if ("[DEPARTURES]" == block.first)
            this->m_configuration.valid &= this->parseDepartures(block.second);
        else if ("[STANDS]" == block.first)
            this->m_configuration.valid &= this->parseStands(block.second);
    }
}

const types::AirportConfiguration& AirportFileFormat::configuration() const {
    return this->m_configuration;
}
