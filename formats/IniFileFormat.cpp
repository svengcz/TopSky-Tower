/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the euroscope file format
 */

#include <fstream>

#include <helper/Exception.h>
#include <helper/String.h>

#include "IniFileFormat.h"

using namespace topskytower;
using namespace topskytower::formats;

IniFileFormat::IniFileFormat(const std::string& filename) :
        m_blocks() {
    std::ifstream stream(filename);
    if (false == stream.is_open())
        throw helper::Exception("IniFileFormat", "File not found: " + filename);

    this->m_blocks.clear();

    std::string line;
    std::string blockName;
    while (std::getline(stream, line)) {
        /* found the next block */
        if (0 == line.rfind("[", 0)) {
            blockName = line;
            this->m_blocks[blockName] = std::vector<std::string>();
        }
        else if (0 != blockName.length() && 0 != line.length()) {
            this->m_blocks[blockName].push_back(line);
        }
    }
}

types::Coordinate IniFileFormat::parsePosition(const std::string& line) {
    auto elements = helper::String::splitString(line, ":");
    if (3 != elements.size())
        throw helper::Exception("IniFileFormat/COORD", "Invalid COORD-entry found");
    return types::Coordinate(elements[2], elements[1]);
}
