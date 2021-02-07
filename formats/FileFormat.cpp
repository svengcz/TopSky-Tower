/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the generic file format
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <fstream>
#include <limits>

#include <formats/FileFormat.h>

using namespace topskytower::formats;

FileFormat::FileFormat() :
        m_errorLine(std::numeric_limits<std::uint32_t>::max()),
        m_errorMessage() { }

void FileFormat::reset() {
    this->m_errorLine = std::numeric_limits<std::uint32_t>::max();
    this->m_errorMessage.clear();
}

bool FileFormat::errorFound() const {
    return std::numeric_limits<std::uint32_t>::max() != this->m_errorLine;
}

std::uint32_t FileFormat::errorLine() const {
    return this->m_errorLine;
}

const std::string& FileFormat::errorMessage() const {
    return this->m_errorMessage;
}
