/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the settings file format
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <fstream>

#include <formats/SettingsFileFormat.h>
#include <helper/String.h>
#include <types/Aircraft.h>

using namespace topskytower;
using namespace topskytower::formats;

bool SettingsFileFormat::parseColor(const std::string& block, std::uint8_t color[3], std::uint32_t line) {
    auto split = helper::String::splitString(block, ",");
    if (3 != split.size()) {
        this->m_errorLine = line;
        this->m_errorMessage = "Invalid color configuration";
        return false;
    }

    color[0] = static_cast<std::uint8_t>(std::atoi(split[0].c_str()));
    color[1] = static_cast<std::uint8_t>(std::atoi(split[1].c_str()));
    color[2] = static_cast<std::uint8_t>(std::atoi(split[2].c_str()));

    return true;
}

SettingsFileFormat::SettingsFileFormat(const std::string& filename) :
        FileFormat(),
        m_filename(filename) { }

bool SettingsFileFormat::parse(types::SystemConfiguration& config) {
    types::Aircraft::WTC defaultWtc = types::Aircraft::WTC::Medium;
    config.valid = true;

    std::ifstream stream(this->m_filename);
    if (false == stream.is_open()) {
        this->m_errorMessage = "Unable to open the configuration file";
        this->m_errorLine = 0;
        config.valid = false;
        return false;
    }

    std::string line;
    std::uint32_t lineOffset = 0;
    while (std::getline(stream, line)) {
        bool retval = true;
        std::string value;

        lineOffset += 1;

        /* skip a new line */
        if (0 == line.length())
            continue;

        auto entry = helper::String::splitString(line, "=");
        if (2 > entry.size()) {
            this->m_errorLine = lineOffset;
            this->m_errorMessage = "Invalid configuration entry";
            config.valid = false;
            return false;
        }
        else if (2 < entry.size()) {
            for (std::size_t idx = 1; idx < entry.size() - 1; ++idx)
                value += entry[idx] + "=";
            value += entry.back();
        }
        else {
            value = entry[1];
        }

        /* found an invalid line */
        if (0 == value.length()) {
            this->m_errorLine = lineOffset;
            this->m_errorMessage = "Invalid entry";
            return false;
        }

        if ("UI_BackgroundColor" == entry[0]) {
            retval = this->parseColor(value, config.uiBackgroundColor, lineOffset);
        }
        else if ("UI_ForegroundColor" == entry[0]) {
            retval = this->parseColor(value, config.uiForegroundColor, lineOffset);
        }
        else if ("UI_BackgroundActiveColor" == entry[0]) {
            retval = this->parseColor(value, config.uiBackgroundActiveColor, lineOffset);
        }
        else if ("UI_ForegroundActiveColor" == entry[0]) {
            retval = this->parseColor(value, config.uiForegroundActiveColor, lineOffset);
        }
        else if ("UI_ScreenClickColor" == entry[0]) {
            retval = this->parseColor(value, config.uiScreenClickColor, lineOffset);
        }
        else if ("UI_FontFamily" == entry[0]) {
            config.fontFamily = value;
        }
        else if ("UI_FontSize" == entry[0]) {
            config.fontSize = static_cast<float>(std::atof(value.c_str()));
        }
        else if ("UI_NTZColor" == entry[0]) {
            retval = this->parseColor(value, config.uiNtzColor, lineOffset);
        }
        else if ("HTTP_HoppiesURL" == entry[0]) {
            config.hoppiesUrl = value;
        }
        else if ("HTTP_VersionCheckURL" == entry[0]) {
            config.versionCheckUrl = value;
        }
        else if ("HTTP_NotamURL" == entry[0]) {
            config.notamUrl = value;
        }
        else if ("HTTP_NotamsMarkerStart" == entry[0]) {
            config.notamMarkerStart = value;
        }
        else if ("HTTP_NotamsMarkerEnd" == entry[0]) {
            config.notamMarkerEnd = value;
        }
        else if ("SYS_TrackingOnGround" == entry[0]) {
            config.trackingOnGround = '0' != value[0];
        }
        else if ("SYS_SurveillanceVisualizationDuration" == entry[0]) {
            config.surveillanceVisualizationDuration = static_cast<float>(std::atoi(value.c_str())) * types::second;
        }
        else if ("SURV_RDF_Active" == entry[0]) {
            config.rdfActive = '0' != value[0];
        }
        else if ("SURV_RDF_Radius" == entry[0]) {
            config.rdfRadius = static_cast<float>(std::atoi(value.c_str()));
        }
        else if ("SURV_RDF_NonConflictColor" == entry[0]) {
            retval = this->parseColor(value, config.rdfNonConflictColor, lineOffset);
        }
        else if ("SURV_RDF_ConflictColor" == entry[0]) {
            retval = this->parseColor(value, config.rdfConflictColor, lineOffset);
        }
        else if ("SURV_FlightPlanCheckEvenOdd" == entry[0]) {
            config.flightPlanCheckEvenOdd = '0' != value[0];
        }
        else if ("SURV_FlightPlanCheckNav" == entry[0]) {
            config.flightPlanCheckNavigation = '0' != value[0];
        }
        else if ("SYS_DistanceStandAssignment" == entry[0]) {
            config.standAssociationDistance = static_cast<float>(std::atoi(value.c_str())) * types::nauticmile;
        }
        else if ("SURV_ARIWS_Active" == entry[0]) {
            config.ariwsActive = '0' != value[0];
        }
        else if ("SURV_ARIWS_DistanceDeadband" == entry[0]) {
            config.ariwsDistanceDeadband = static_cast<float>(std::atoi(value.c_str())) * types::metre;
        }
        else if ("SURV_ARIWS_MaxDistance" == entry[0]) {
            config.ariwsMaximumDistance = static_cast<float>(std::atoi(value.c_str())) * types::metre;
        }
        else if ("SURV_CMAC_Active" == entry[0]) {
            config.cmacActive = '0' != value[0];
        }
        else if ("SURV_CMAC_MinDistance" == entry[0]) {
            config.cmacMinimumDistance = static_cast<float>(std::atoi(value.c_str())) * types::metre;
        }
        else if ("SURV_CMAC_CycleReset" == entry[0]) {
            config.cmacCycleReset = static_cast<std::uint8_t>(std::atoi(value.c_str()));
        }
        else if ("SURV_STCD_Active" == entry[0]) {
            config.stcdActive = '0' != value[0];
        }
        else if ("SURV_MTCD_Active" == entry[0]) {
            config.mtcdActive = '0' != value[0];
        }
        else if ("SURV_MTCD_DepartureModelUnknown" == entry[0]) {
            switch (value[0]) {
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
                this->m_errorLine = lineOffset;
                this->m_errorMessage = "Invalid WTC category";
                return false;
            }
        }
        else if ("SURV_MTCD_DepartureSpeedV2" == entry[0]) {
            retval = this->parseDepartureModelParameters(value, config.mtcdDepartureSpeedV2, types::knot, lineOffset);
        }
        else if ("SURV_MTCD_DepartureCruiseSpeed" == entry[0]) {
            retval = this->parseDepartureModelParameters(value, config.mtcdDepartureCruiseTAS, types::knot, lineOffset);
        }
        else if ("SURV_MTCD_DepartureClimbRate" == entry[0]) {
        retval = this->parseDepartureModelParameters(value, config.mtcdDepartureClimbRates, types::feet / types::minute, lineOffset);
        }
        else if ("SURV_MTCD_DepartureAccelerationAlt" == entry[0]) {
            config.mtcdDepartureAccelerationAlt = static_cast<float>(std::atoi(value.c_str())) * types::feet;
        }
        else if ("SURV_MTCD_DepartureAcceleration" == entry[0]) {
            config.mtcdDepartureAcceleration =
                static_cast<float>(std::atof(value.c_str())) * (types::metre / (types::second * types::second));
        }
        else if ("SURV_MTCD_DepartureSpeedBelowFL100" == entry[0]) {
            config.mtcdDepartureSpeedBelowFL100 = static_cast<float>(std::atoi(value.c_str())) * types::knot;
        }
        else if ("SURV_MTCD_VerticalSpacing" == entry[0]) {
            config.mtcdVerticalSeparation = static_cast<float>(std::atoi(value.c_str())) * types::feet;
        }
        else if ("SURV_MTCD_HorizontalSpacing" == entry[0]) {
            config.mtcdHorizontalSeparation = static_cast<float>(std::atoi(value.c_str())) * types::nauticmile;
        }
        else if ("SURV_MTCD_VerticalSpacingSameDestination" == entry[0]) {
            config.mtcdVerticalSeparationSameDestination = static_cast<float>(std::atoi(value.c_str())) * types::feet;
        }
        else if ("SURV_STCD_Active" == entry[0]) {
            config.stdcActive = '0' != value[0];
        }
        else {
            this->m_errorLine = lineOffset;
            this->m_errorMessage = "Unknown entry: " + entry[0];
            return false;
        }

        if (false == retval)
            return false;
    }

    if (0 == lineOffset) {
        this->m_errorLine = 0;
        this->m_errorMessage = "No data found in TopSkyTowerSettings.txt";
        return false;
    }

    /* copy the MTCA-default values of the departure model */
    config.mtcdDepartureSpeedV2[static_cast<int>(types::Aircraft::WTC::Unknown)] =
        config.mtcdDepartureSpeedV2[static_cast<int>(defaultWtc)];
    config.mtcdDepartureCruiseTAS[static_cast<int>(types::Aircraft::WTC::Unknown)] =
        config.mtcdDepartureCruiseTAS[static_cast<int>(defaultWtc)];
    config.mtcdDepartureClimbRates[static_cast<int>(types::Aircraft::WTC::Unknown)] =
        config.mtcdDepartureClimbRates[static_cast<int>(defaultWtc)];

    return true;
}
