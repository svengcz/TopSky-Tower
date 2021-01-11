/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the settings file format
 */

#include <fstream>

#include <formats/SettingsFileFormat.h>
#include <helper/String.h>
#include <types/Aircraft.h>

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
    types::Aircraft::WTC defaultWtc = types::Aircraft::WTC::Medium;
    config.valid = true;

    std::ifstream stream(this->m_filename);
    if (false == stream.is_open()) {
        config.valid = false;
        return;
    }

    std::string line;
    while (std::getline(stream, line)) {
        /* skip a new line */
        if (0 == line.length())
            continue;

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
        else if ("UI_FontFamily" == entry[0]) {
            config.fontFamily = entry[1];
        }
        else if ("UI_FontSize" == entry[0]) {
            config.fontSize = static_cast<float>(std::atof(entry[1].c_str()));
        }
        else if ("SYS_TrackingOnGround" == entry[0]) {
            config.trackingOnGround = '0' != entry[1][0];
        }
        else if ("SURV_FlightPlanCheckEvenOdd" == entry[0]) {
            config.flightPlanCheckEvenOdd = '0' != entry[1][0];
        }
        else if ("SYS_DistanceStandAssignment" == entry[0]) {
            config.standAssociationDistance = static_cast<float>(std::atoi(entry[1].c_str())) * types::nauticmile;
        }
        else if ("SURV_ARIWS_Active" == entry[0]) {
            config.ariwsActive = '0' != entry[1][0];
        }
        else if ("SURV_ARIWS_DistanceDeadband" == entry[0]) {
            config.ariwsDistanceDeadband = static_cast<float>(std::atoi(entry[1].c_str())) * types::metre;
        }
        else if ("SURV_MTCA_DepartureModelUnknown" == entry[0]) {
            switch (entry[1][0]) {
            case 'L':
                defaultWtc = types::Aircraft::WTC::Light;
                break;
            case 'M':
                defaultWtc = types::Aircraft::WTC::Medium;
                break;
            case 'H':
                defaultWtc = types::Aircraft::WTC::Heavy;
                break;
            case 'J':
                defaultWtc = types::Aircraft::WTC::Super;
                break;
            default:
                break;
            }
        }
        else if ("MTCA_DepartureSpeedV2" == entry[0]) {
            SettingsFileFormat::parseDepartureModelParameters(entry[1], config.departureSpeedV2, types::knot);
        }
        else if ("MTCA_DepartureCruiseSpeed" == entry[0]) {
            SettingsFileFormat::parseDepartureModelParameters(entry[1], config.departureCruiseTAS, types::knot);
        }
        else if ("MTCA_DepartureClimbRate" == entry[0]) {
            SettingsFileFormat::parseDepartureModelParameters(entry[1], config.departureClimbRates, types::feet / types::minute);
        }
        else if ("MTCA_DepartureAccelerationAlt" == entry[0]) {
            config.departureAccelerationAlt = static_cast<float>(std::atoi(entry[1].c_str())) * types::feet;
        }
        else if ("MTCA_DepartureAcceleration" == entry[0]) {
            config.departureAcceleration =
                static_cast<float>(std::atof(entry[1].c_str())) * (types::metre / (types::second * types::second));
        }
        else if ("MTCA_DepartureSpeedBelowFL100" == entry[0]) {
            config.departureSpeedBelowFL100 = static_cast<float>(std::atoi(entry[1].c_str())) * types::knot;
        }
    }

    /* copy the MTCA-default values of the departure model */
    config.departureSpeedV2[static_cast<int>(types::Aircraft::WTC::Unknown)] = config.departureSpeedV2[static_cast<int>(defaultWtc)];
    config.departureCruiseTAS[static_cast<int>(types::Aircraft::WTC::Unknown)] = config.departureCruiseTAS[static_cast<int>(defaultWtc)];
    config.departureClimbRates[static_cast<int>(types::Aircraft::WTC::Unknown)] = config.departureClimbRates[static_cast<int>(defaultWtc)];
}
