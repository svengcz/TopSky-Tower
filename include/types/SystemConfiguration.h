/*
 * @brief Defines a system configuration
 * @file types/SystemConfiguration.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <string>

#include <types/Quantity.hpp>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes the system-wide configuration
         * @ingroup types
         */
        struct SystemConfiguration {
            bool                valid;                                 /**< Marks if the configuration is valid */
            std::string         hoppiesCode;                           /**< Defines Hoppie's logon-code */
            std::uint8_t        uiBackgroundColor[3];                  /**< Defines the background color of the UI elements */
            std::uint8_t        uiForegroundColor[3];                  /**< Defines the foreground color of the UI elements */
            std::uint8_t        uiBackgroundActiveColor[3];            /**< Defines the background color of active UI elements */
            std::uint8_t        uiForegroundActiveColor[3];            /**< Defines the foreground color of active UI elements */
            std::uint8_t        uiScreenClickColor[3];                 /**< Defines the color which is used for click events on the RADAR screen */
            std::uint8_t        uiNtzColor[3];                         /**< Defines the NTZ color */
            std::string         hoppiesUrl;                            /**< Defines the Hoppies URL to enable PDC */
            std::string         versionCheckUrl;                       /**< Defines the version check URL */
            std::string         notamUrl;                              /**< Defines the URL to receive airport-specific NOTAMs */
            std::string         notamMarkerStart;                      /**< Defines the markers that enclose a NOTAM message */
            std::string         notamMarkerEnd;                        /**< Defines the markers that enclose a NOTAM message */
            std::string         fontFamily;                            /**< Defines the font family of the visualizations */
            float               fontSize;                              /**< Defines the font size of the visualizations */
            bool                trackingOnGround;                      /**< Defines if the ground controllers track flights */
            bool                flightPlanCheckEvenOdd;                /**< Defines if the generic even-odd checks are performed */
            bool                flightPlanCheckNavigation;             /**< Defines if the flight plan checker tests for RNAV-capabilities */
            types::Length       standAssociationDistance;              /**< Defines the maximum distance to assign automatically an aircraft to a stand */
            types::Time         surveillanceVisualizationDuration;     /**< Defines the duration of visualization durations */
            bool                ariwsActive;                           /**< Defines if ARIWS is active or not */
            types::Length       ariwsDistanceDeadband;                 /**< Defines the distance in which the RIW is suppressed around the holding point */
            types::Length       ariwsMaximumDistance;                  /**< Defines the maximum distance to check if the flight is on the runway */
            bool                cmacActive;                            /**< Defines if CMAC is active or not */
            std::uint8_t        cmacCycleReset;                        /**< Defines after how many non-moving cycles the system for a flight resets */
            types::Length       cmacMinimumDistance;                   /**< Defines the minimum distance between the reference position and the new position to estimate the CMA */
            bool                stcdActive;                            /**< Defines if STCD is active or not */
            bool                mtcdActive;                            /**< Defines if MTCD is active or not */
            types::Velocity     mtcdDepartureSpeedV2[5];               /**< Defines the V2 speeds for all WTCs */
            types::Length       mtcdDepartureAccelerationAlt;          /**< Defines the acceleration altitude */
            types::Acceleration mtcdDepartureAcceleration;             /**< Defines the average aircrafts acceleration */
            types::Velocity     mtcdDepartureSpeedBelowFL100;          /**< Defines the velocity below FL100 */
            types::Velocity     mtcdDepartureCruiseTAS[5];             /**< Defines the true air speed during cruise for all WTCs */
            types::Velocity     mtcdDepartureClimbRates[5];            /**< Defines the climb rates for all WTCs */
            types::Length       mtcdVerticalSeparation;                /**< Defines the vertical separation between flights */
            types::Length       mtcdVerticalSeparationSameDestination; /**< Defines the vertical separation between flights with the same destination */
            types::Length       mtcdHorizontalSeparation;              /**< Defines the horizontal separation between flights */
            bool                stdcActive;                            /**< Defines if STCD is active or not */

            /**
             * @brief Creates an empty and uninitialized system configuration
             */
            SystemConfiguration() :
                    valid(false),
                    hoppiesCode(),
                    uiBackgroundColor{ 50, 50, 50 },
                    uiForegroundColor{ 150, 150, 150, },
                    uiBackgroundActiveColor{ 0, 150, 0 },
                    uiForegroundActiveColor{ 100, 100, 100 },
                    uiScreenClickColor{ 0, 0, 200 },
                    uiNtzColor{ 200, 0, 0 },
                    hoppiesUrl("http://www.hoppie.nl/acars/system/connect.html?logon=%LOGON%&from=%SENDER%"),
                    versionCheckUrl("https://raw.githubusercontent.com/svengcz/Versioning/master/TopSky-Tower.txt"),
                    notamUrl("https://www.notams.faa.gov/PilotWeb/notamRetrievalByICAOAction.do?method=displayByICAOs&reportType=RAW&formatType=DOMESTIC&retrieveLocId=%AIRPORT%&actionType=notamRetrievalByICAOs"),
                    notamMarkerStart("<PRE>"),
                    notamMarkerEnd("</PRE>"),
                    fontFamily(),
                    fontSize(3.2f),
                    trackingOnGround(false),
                    flightPlanCheckEvenOdd(true),
                    flightPlanCheckNavigation(true),
                    standAssociationDistance(10_nm),
                    surveillanceVisualizationDuration(10_s),
                    ariwsActive(true),
                    ariwsDistanceDeadband(50_m),
                    ariwsMaximumDistance(100_m),
                    cmacActive(true),
                    cmacCycleReset(10),
                    cmacMinimumDistance(20_m),
                    stcdActive(true),
                    mtcdActive(true),
                    mtcdDepartureSpeedV2{ 160_kn, 90_kn, 160_kn, 180_kn, 190_kn },
                    mtcdDepartureAccelerationAlt(2000_ft),
                    mtcdDepartureAcceleration(1.8_mps2),
                    mtcdDepartureSpeedBelowFL100(250_kn),
                    mtcdDepartureCruiseTAS{ 270_kn, 120_kn, 270_kn, 290_kn, 290_kn },
                    mtcdDepartureClimbRates{ 2000_ftpmin, 1000_ftpmin, 2000_ftpmin, 2000_ftpmin, 2000_ftpmin },
                    mtcdVerticalSeparation(2000_ft),
                    mtcdVerticalSeparationSameDestination(6000_ft),
                    mtcdHorizontalSeparation(10_nm),
                    stdcActive(true) { }
        };
    }
}
