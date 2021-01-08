/*
 * @brief Defines a system configuration
 * @file types/SystemConfiguration.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>

#include <types/Quantity.hpp>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a controller information
         * @ingroup types
         */
        struct SystemConfiguration {
            bool                valid;                    /**< Marks if the configuration is valid */
            std::string         hoppiesCode;              /**< Defines Hoppie's logon-code */
            std::uint8_t        uiBackgroundColor[3];     /**< Defines the background color of the UI elements */
            std::uint8_t        uiForegroundColor[3];     /**< Defines the foreground color of the UI elements */
            std::string         fontFamily;               /**< Defines the font family of the visualizations */
            float               fontSize;                 /**< Defines the font size of the visualizations */
            bool                trackingOnGround;         /**< Defines if the ground controllers track flights */
            bool                flightPlanCheckEvenOdd;   /**< Defines if the generic even-odd checks are performed */
            types::Length       standAssociationDistance; /**< Defines the maximum distance to assign automatically an aircraft to a stand */
            types::Velocity     departureSpeedV2[5];      /**< Defines the V2 speeds for all WTCs */
            types::Length       departureAccelerationAlt; /**< Defines the acceleration altitude */
            types::Acceleration departureAcceleration;    /**< Defines the average aircrafts acceleration */
            types::Velocity     departureSpeedBelowFL100; /**< Defines the velocity below FL100 */
            types::Velocity     departureCruiseTAS[5];    /**< Defines the true air speed during cruise for all WTCs */
            types::Velocity     departureClimbRates[5];   /**< Defines the climb rates for all WTCs */

            /**
             * @brief Creates an empty and uninitialized system configuration
             */
            SystemConfiguration() :
                    valid(false),
                    hoppiesCode(),
                    uiBackgroundColor{ 50, 50, 50 },
                    uiForegroundColor{ 150, 150, 150, },
                    fontFamily(),
                    fontSize(3.2f),
                    trackingOnGround(false),
                    flightPlanCheckEvenOdd(true),
                    standAssociationDistance(10_nm),
                    departureSpeedV2{ 160_kn, 90_kn, 160_kn, 180_kn, 190_kn },
                    departureAccelerationAlt(2000_ft),
                    departureAcceleration(1.8_mps2),
                    departureSpeedBelowFL100(250_kn),
                    departureCruiseTAS{ 270_kn, 120_kn, 270_kn, 290_kn, 290_kn },
                    departureClimbRates{ 2000_ftpmin, 1000_ftpmin, 2000_ftpmin, 2000_ftpmin, 2000_ftpmin } { }
        };
    }
}
