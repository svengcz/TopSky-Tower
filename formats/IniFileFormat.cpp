/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the euroscope file format
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <fstream>

#include <helper/Exception.h>
#include <helper/String.h>

#include "IniFileFormat.h"

using namespace topskytower;
using namespace topskytower::formats;

IniFileFormat::IniFileFormat(const std::string& filename) :
        FileFormat(),
        m_blocks(),
        m_lineOffsets() {
    std::ifstream stream(filename);
    if (false == stream.is_open()) {
        this->m_errorLine = 0;
        this->m_errorMessage = "File not found!";
        return;
    }

    std::string line;
    std::string blockName;
    std::uint32_t lineOffset = 1;
    while (std::getline(stream, line)) {
        /* found the next block */
        if (0 == line.rfind("[", 0)) {
            blockName = line;
            this->m_blocks[blockName] = std::vector<std::string>();
            this->m_lineOffsets[blockName] = lineOffset;
        }
        else if (0 != blockName.length()) {
            this->m_blocks[blockName].push_back(line);
        }

        lineOffset += 1;
    }
}

types::Coordinate IniFileFormat::parsePosition(const std::string& line) {
    auto elements = helper::String::splitString(line, ":");
    if (3 != elements.size())
        throw helper::Exception("IniFileFormat/COORD", "Invalid COORD-entry found");
    return types::Coordinate(elements[2], elements[1]);
}
