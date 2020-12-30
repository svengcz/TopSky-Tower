/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the settings file format
 */

#include <fstream>

#include <helper/String.h>
#include <formats/SettingsFileFormat.h>

using namespace topskytower;
using namespace topskytower::formats;

void SettingsFileFormat::parseColor(const std::string& block, std::uint8_t color[3]) {
    auto split = helper::String::splitString(block, ",");
    if (3 != split.size())
        return;

    color[0] = static_cast<std::uint8_t>(std::atoi(split[0].c_str()));
    color[1] = static_cast<std::uint8_t>(std::atoi(split[1].c_str()));
    color[2] = static_cast<std::uint8_t>(std::atoi(split[2].c_str()));
}

SettingsFileFormat::SettingsFileFormat(const std::string& filename) :
        m_filename(filename) { }

void SettingsFileFormat::parse(types::SystemConfiguration& config) const {
    config.valid = true;

    std::ifstream stream(this->m_filename);
    if (false == stream.is_open()) {
        config.valid = false;
        return;
    }

    std::string line;
    while (std::getline(stream, line)) {
        auto entry = helper::String::splitString(line, "=");
        if (2 != entry.size()) {
            config.valid = false;
            return;
        }

        if ("UI_BackgroundColor" == entry[0]) {
            SettingsFileFormat::parseColor(entry[1], config.uiBackgroundColor);
        }
        else if ("UI_ForegroundColor" == entry[0]) {
            SettingsFileFormat::parseColor(entry[1], config.uiForegroundColor);
        }
        else {
            config.valid = false;
            return;
        }
    }
}