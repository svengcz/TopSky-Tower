/*
 * @brief Defines the system file format
 * @file formats/SettingsFileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <formats/FileFormat.h>
#include <helper/String.h>
#include <types/Aircraft.h>
#include <types/SystemConfiguration.h>

namespace topskytower {
    namespace formats {
        /**
         * @brief Defines the parser of system configurations
         * @ingroup format
         *
         * The settings file format is the format that contains generic and system-wide configurations.
         * All configuration entries need to be stored in 'TopSkyTowerSettings.txt'.
         *
         * The following entries of the configuration exist:
         * <table>
         *   <caption id="multi_row">Setting entries</caption>
         *   <tr>
         *     <th>Name</th><th>Description</th><th>Default value</th><th>Unit</th>
         *   </tr>
         *   <tr>
         *     <td>UI_FontFamily</td>
         *     <td>Defines the font that is used in the UI elements. It does not have a fallback.</td>
         *     <td></td><td>None</td>
         *   </tr>
         *   <tr>
         *     <td>UI_FontSize</td>
         *     <td>Defines the font size that is used in the UI elements.</td>
         *     <td>3.2</td><td>Pt</td>
         *   </tr>
         *   <tr>
         *     <td>UI_BackgroundColor</td>
         *     <td>Defines the background color of TopSky-Tower windows.</td>
         *     <td>50, 50, 50</td><td>R,G,B</td>
         *   </tr>
         *   <tr>
         *     <td>UI_ForegroundColor</td>
         *     <td>Defines the foreground color of TopSky-Tower windows.</td>
         *     <td>150, 150, 150</td><td>R,G,B</td>
         *   </tr>
         *   <tr>
         *     <td>UI_BackgroundActiveColor</td>
         *     <td>Defines the background color of TopSky-Tower windows for active elements.</td>
         *     <td>0, 150, 0</td><td>R,G,B</td>
         *   </tr>
         *   <tr>
         *     <td>UI_ForegroundActiveColor</td>
         *     <td>Defines the foreground color of TopSky-Tower windows for active elements.</td>
         *     <td>100, 100, 100</td><td>R,G,B</td>
         *   </tr>
         *   <tr>
         *     <td>UI_ScreenClickColor</td>
         *     <td>Defines the color that is used to mark clickable areas on the screen.</td>
         *     <td>0, 0, 200</td><td>R,G,B</td>
         *   </tr>
         *   <tr>
         *     <td>UI_NTZColor</td>
         *     <td>Defines the color to visualize NTZ areas.</td>
         *     <td>200, 0, 0</td><td>R,G,B</td>
         *   </tr>
         *   <tr>
         *     <td>HTTP_HoppiesURL</td>
         *     <td>Defines the Hoppies URL. It requires the marker %LOGON% for the Hoppies code and %SENDER% for the station.</td>
         *     <td>http://www.hoppie.nl/acars/system/connect.html?logon=%LOGON%&from=%SENDER%</td><td></td>
         *   </tr>
         *   <tr>
         *     <td>HTTP_VersionCheckURL</td>
         *     <td>Defines the URL which contains TopSky-Tower version information</td>
         *     <td>https://raw.githubusercontent.com/svengcz/TopSky-Tower/master/version.txt</td><td></td>
         *   </tr>
         *   <tr>
         *     <td>HTTP_NotamURL</td>
         *     <td>Defines the URL to receive current NOTAMs. The %AIRPORT% marker must be defined for the airport</td>
         *     <td>https://www.notams.faa.gov/PilotWeb/notamRetrievalByICAOAction.do?method=displayByICAOs&reportType=RAW&formatType=DOMESTIC&retrieveLocId=%AIRPORT%&actionType=notamRetrievalByICAOs</td><td></td>
         *   </tr>
         *   <tr>
         *     <td>HTTP_NotamsMarkerStart</td>
         *     <td>Defines the start marker of a NOTAM message</td>
         *     <td>\<PRE\></td><td></td>
         *   </tr>
         *   <tr>
         *     <td>HTTP_NotamsMarkerEnd</td>
         *     <td>Defines the end marker of a NOTAM message</td>
         *     <td>\</PRE\></td><td></td>
         *   </tr>
         *   <tr>
         *     <td>SYS_TrackingOnGround</td>
         *     <td>Defines if the ground tracks or not.</td>
         *     <td>1</td><td>Boolean</td>
         *   </tr>
         *   <tr>
         *     <td>SYS_DistanceStandAssignment</td>
         *     <td>Defines the distance between the aircraft and the center of the airport below which a stand is assigned.</td>
         *     <td>10</td><td>NM</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_RDF_Active</td>
         *     <td>Defines if RDF is active or not. Euroscope needs to be restarted, if this entry is changed</td>
         *     <td>1</td><td>Boolean</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_RDF_Radius</td>
         *     <td>Defines the radius of the transmission circle.</td>
         *     <td>20</td><td>Pixelstd>
         *   </tr>
         *   <tr>
         *     <td>SURV_RDF_NonConflictColor</td>
         *     <td>Defines the visualization color if only one aircraft is transmitting.</td>
         *     <td>200,200,200</td><td>RGB</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_RDF_ConflictColor</td>
         *     <td>Defines the visualization color if more than one aircraft is transmitting.</td>
         *     <td>200,0,2000td><td>RGB</td>
         *   </tr>
         *   <tr>
         *     <td>SYS_SurveillanceVisualizationDuration</td>
         *     <td>Defines the visualization duration of surveillance results.</td>
         *     <td>10</td><td>Seconds</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_FlightPlanCheckEvenOdd</td>
         *     <td>Defines if the flight plan checker needs to check the destination independent even/odd-rule.</td>
         *     <td>1</td><td>Boolean</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_FlightPlanCheckNav</td>
         *     <td>Defines if the flight plan checker needs to check the RNAV-capabilities.</td>
         *     <td>1</td><td>Boolean</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_ARIWS_Active</td>
         *     <td>Defines if ARIWS is active.</td>
         *     <td>1</td><td>Boolean</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_ARIWS_DistanceDeadband</td>
         *     <td>Defines the distance between the flight and the holding point in which the RIW is suppressed.</td>
         *     <td>50</td><td>Metres</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_ARIWS_MaxDistance</td>
         *     <td>Defines the maximum distance between the flight and the holding point to check if RIW is valid.</td>
         *     <td>50</td><td>Metres</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_CMAC_Active</td>
         *     <td>Defines if CMAC is active.</td>
         *     <td>1</td><td>Boolean</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_CMAC_CycleReset</td>
         *     <td>Defines the number of non-moving cycles after which the status resets for a flight.</td>
         *     <td>10</td><td>None</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_CMAC_MinDistance</td>
         *     <td>Defines the minimum distance between the reference position and the current position to evaluate the CMAC.</td>
         *     <td>20</td><td>Metres</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_STCD_Active</td>
         *     <td>Defines if STCD is active.</td>
         *     <td>1</td><td>Boolean</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCD_Active</td>
         *     <td>Defines if MTCD is active.</td>
         *     <td>1</td><td>Boolean</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCD_DepartureModelUnknown</td>
         *     <td>Defines which model is used if the WTC category of a flight is unknown.</td>
         *     <td>M</td><td>WTC</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCD_DepartureSpeedV2</td>
         *     <td>
         *       Defines the average V2 speeds of departing aircrafts based on WTC categories.
         *       The speeds for the categories are seperated by ',' and four values (L, M, H, J) are required.
         *     </td>
         *     <td>90, 160, 180, 190</td><td>Knot</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCD_DepartureCruiseSpeed</td>
         *     <td>
         *       Defines the average cruise speeds of departing aircrafts based on WTC categories that are above FL100.
         *       The speeds for the categories are seperated by ',' and four values (L, M, H, J) are required.
         *     </td>
         *     <td>120, 270, 290, 290</td><td>Knot</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCD_DepartureClimbRate</td>
         *     <td>
         *       Defines the average climb rates of departing aircrafts based on WTC categories that are above the acceleration altitude.
         *       The climb rates for the categories are seperated by ',' and four values (L, M, H, J) are required.
         *     </td>
         *     <td>1000, 2000, 2000, 2000</td><td>Feet per minute</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCD_DepartureAccelerationAlt</td>
         *     <td>Defines the acceleration altitude above which the aircraft climbs with the defined climb rate.</td>
         *     <td>2000</td><td>Feet</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCD_DepartureAcceleration</td>
         *     <td>Defines the acceleration of all aircrafts with which the speed increases.</td>
         *     <td>1.8</td><td>Meter per square-second</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCD_DepartureSpeedBelowFL100</td>
         *     <td>Defines the maximum speed below FL100.</td>
         *     <td>250</td><td>Knot</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCD_VerticalSpacing</td>
         *     <td>Defines the vertical spacing between two departures.</td>
         *     <td>2000</td><td>Feet</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCD_HorizontalSpacing</td>
         *     <td>Defines the horizontal spacing between two departures.</td>
         *     <td>10</td><td>NM</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCD_VerticalSpacingSameDestination</td>
         *     <td>Defines the vertical spacing between two departures with the same destination.</td>
         *     <td>6000</td><td>Knot</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_STCD_Active</td>
         *     <td>Defines if STCD is active.</td>
         *     <td>1</td><td>Boolean</td>
         *   </tr>
         * </table>
         */
        class SettingsFileFormat : public FileFormat {
        private:
#ifndef DOXYGEN_IGNORE
            std::string m_filename;

            bool parseColor(const std::string& block, std::uint8_t color[3], std::uint32_t line);
            template <typename T>
            bool parseDepartureModelParameters(const std::string& block, T parameters[5], const T& unit, std::uint32_t line) {
                auto split = helper::String::splitString(block, ",");
                /* all WTC categories need values */
                if (4 != split.size()) {
                    this->m_errorLine = line;
                    this->m_errorMessage = "Invalid model configuration";
                    return false;
                }

                parameters[static_cast<int>(types::Aircraft::WTC::Light)] = static_cast<float>(std::atof(split[0].c_str())) * unit;
                parameters[static_cast<int>(types::Aircraft::WTC::Medium)] = static_cast<float>(std::atof(split[1].c_str())) * unit;
                parameters[static_cast<int>(types::Aircraft::WTC::Heavy)] = static_cast<float>(std::atof(split[2].c_str())) * unit;
                parameters[static_cast<int>(types::Aircraft::WTC::Super)] = static_cast<float>(std::atof(split[3].c_str())) * unit;

                return true;
            }

        public:
            /**
             * @brief Parses a settings configuration
             * If the file does not exist, the constructor throws an exeption.
             * @param[in] filename The filename of the configuration
             */
            SettingsFileFormat(const std::string& filename);

            /**
             * @brief Parses the set configuration file
             * @param[out] config The resulting configuration
             * @return True if the configuration file was valid, else false
             */
            bool parse(types::SystemConfiguration& config);
#endif
        };
    }
}
