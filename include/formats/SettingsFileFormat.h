/*
 * @brief Defines the system file format
 * @file formats/SettingsFileFormat.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

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
         *     <td>SURV_FlightPlanCheckEvenOdd</td>
         *     <td>Defines if the flight plan checker needs to check the destination independent even/odd-rule.</td>
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
         *     <td>SURV_MTCA_DepartureModelUnknown</td>
         *     <td>Defines which model is used if the WTC category of a flight is unknown.</td>
         *     <td>M</td><td>WTC</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCA_DepartureSpeedV2</td>
         *     <td>
         *       Defines the average V2 speeds of departing aircrafts based on WTC categories.
         *       The speeds for the categories are seperated by ',' and four values (L, M, H, J) are required.
         *     </td>
         *     <td>90, 160, 180, 190</td><td>Knot</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCA_DepartureCruiseSpeed</td>
         *     <td>
         *       Defines the average cruise speeds of departing aircrafts based on WTC categories that are above FL100.
         *       The speeds for the categories are seperated by ',' and four values (L, M, H, J) are required.
         *     </td>
         *     <td>120, 270, 290, 290</td><td>Knot</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCA_DepartureClimbRate</td>
         *     <td>
         *       Defines the average climb rates of departing aircrafts based on WTC categories that are above the acceleration altitude.
         *       The climb rates for the categories are seperated by ',' and four values (L, M, H, J) are required.
         *     </td>
         *     <td>1000, 2000, 2000, 2000</td><td>Feet per minute</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCA_DepartureAccelerationAlt</td>
         *     <td>Defines the acceleration altitude above which the aircraft climbs with the defined climb rate.</td>
         *     <td>2000</td><td>Feet</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCA_DepartureAcceleration</td>
         *     <td>Defines the acceleration of all aircrafts with which the speed increases.</td>
         *     <td>1.8</td><td>Meter per square-second</td>
         *   </tr>
         *   <tr>
         *     <td>SURV_MTCA_DepartureSpeedBelowFL100</td>
         *     <td>Defines the maximum speed below FL100.</td>
         *     <td>250</td><td>Knot</td>
         *   </tr>
         * </table>
         */
        class SettingsFileFormat {
        private:
            std::string m_filename;

            static void parseColor(const std::string& block, std::uint8_t color[3]);
            template <typename T>
            static void parseDepartureModelParameters(const std::string& block, T parameters[5], const T& unit) {
                auto split = helper::String::splitString(block, ",");
                /* all WTC categories need values */
                if (4 != split.size())
                    return;

                parameters[static_cast<int>(types::Aircraft::WTC::Light)] = static_cast<float>(std::atof(split[0].c_str())) * unit;
                parameters[static_cast<int>(types::Aircraft::WTC::Medium)] = static_cast<float>(std::atof(split[1].c_str())) * unit;
                parameters[static_cast<int>(types::Aircraft::WTC::Heavy)] = static_cast<float>(std::atof(split[2].c_str())) * unit;
                parameters[static_cast<int>(types::Aircraft::WTC::Super)] = static_cast<float>(std::atof(split[3].c_str())) * unit;
            }

        public:
            /**
             * @brief Parses an airport configuration
             * If the file does not exist, the constructor throws an exeption.
             * @param[in] filename The filename of the configuration
             */
            SettingsFileFormat(const std::string& filename);

            /**
             * @brief Parses the set configuration file
             * @param[out] config The resulting configuration
             */
            void parse(types::SystemConfiguration& config) const;
        };
    }
}
