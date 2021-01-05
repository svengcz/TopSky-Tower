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

            if (true == this->parseSid(split, sid))
                this->m_configuration.sids[sid.name] = sid;
        }
        else if ("CSTR" == split[0]) {
            types::DestinationConstraint constraint = {
                "",
                false,
                0_ft,
                99000_ft
            };

            if (true == this->parseConstraint(split, constraint))
                this->m_configuration.destinationConstraints.push_back(constraint);
        }
    }

    return true;
}

AirportFileFormat::AirportFileFormat(const std::string& filename) :
        m_configuration() {
    IniFileFormat file(filename);

    for (const auto& block : std::as_const(file.m_blocks)) {
        this->m_configuration.valid = true;

        if ("[DEPARTURES]" == block.first)
            this->m_configuration.valid &= this->parseDepartures(block.second);
    }
}

const types::AirportConfiguration& AirportFileFormat::configuration() const {
    return this->m_configuration;
}
