/*
 * @brief Defines a NOTAM control system
 * @file management/NotamControl.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#ifndef DOXYGEN_IGNORE

#include <types/Coordinate.h>

namespace topskytower {
    namespace management {
        using NotamTimePoint = std::chrono::system_clock::time_point; /**< Defines a more readable type for the time_point */

        /**
         * @brief Defines the NOTAM categories based on the FAA releases
         */
        enum class NotamCategory {
            Unknown             = 0,  /**< An unknown NOTAM */
            Other               = 1,  /**< Non-airport specific */
            MovementArea        = 2,  /**< Movement area specific */
            BearingStrength     = 3,  /**< Bearing strength of landing area */
            Clearway            = 4,  /**< Clearway for specific runway */
            DeclaredDistances   = 5,  /**< Declared distances for specific runway */
            TaxiGuidance        = 6,  /**< Taxi guidance system */
            RunwayArrestingGear = 7,  /**< Runway arresting gear for specific runway */
            Parking             = 8,  /**< Parking area */
            DaylightMarkings    = 9,  /**< Daylight markings for threshold or centerline */
            Apron               = 10, /**< Apron specific */
            Stopbar             = 11, /**< Stopbar for specific runway */
            Stands              = 12, /**< Aircraft stand specific */
            Runway              = 13, /**< Runway for specific runway */
            Stopway             = 14, /**< Stopway for specific runway */
            Threshold           = 15, /**< Threshold for specific runway */
            RunwayTurningBay    = 16, /**< Runway turning bay for specific runway */
            Strip               = 17, /**< Strip or shoulder for specific runway */
            Taxiway             = 18, /**< Taxiway specific */
            RapidExit           = 19  /**< Rapid exit taxiway */
        };

        /**
         * @brief Defines the interpetation status of the NOTAM
         */
        enum class NotamInterpreterState {
            Pending = 0, /**< The interpretation is pending */
            Ignored = 1, /**< The NOTAM is ignored */
            Failed  = 2, /**< The interpreter failed */
            Success = 3  /**< The interpreter was successful and*/
        };

        /**
         * @brief Defines the activation state of the NOTAM
         */
        enum class NotamActiveState {
            Inactive  = 0, /**< The NOTAM is inactive */
            Active    = 1, /**< The NOTAM is active */
            Automatic = 2  /**< The NOTAM is active or inactive based on the start and end time */
        };

        /**
         * @brief Defines the parsed NOTAM information
         */
        struct NotamInformation {
            std::string       fir;           /**< Relevant FIR */
            std::string       code;          /**< The Q-code of the information */
            std::uint8_t      flightRule;    /**< Bitmask for the flight rule of the NOTAM */
            std::string       purpose;       /**< Purpose of the NOTAM */
            std::string       scope;         /**< Scope of the NOTAM */
            types::Length     lowerAltitude; /**< The lower border of the NOTAM handled area */
            types::Length     upperAltitude; /**< The upper border of the NOTAM handled area */
            types::Coordinate coordinate;    /**< The coordinate of the of the NOTAM */
            types::Length     radius;        /**< The active radius of the NOTAM */

            NotamInformation() :
                    fir(),
                    code(),
                    flightRule(0),
                    purpose(),
                    scope(),
                    lowerAltitude(),
                    upperAltitude(),
                    coordinate(),
                    radius() { }
        };

        /**
         * @brief Defines a parsed NOTAM
         */
        struct Notam {
            NotamActiveState      activationState;  /**< The activation state */
            NotamInterpreterState interpreterState; /**< The interpretation state */
            std::string           title;            /**< The NOTAM's title */
            NotamCategory         category;         /**< The NOTAM's category */
            NotamInformation      information;      /**< The NOTAM's information */
            NotamTimePoint        startTime;        /**< The NOTAM's start time */
            NotamTimePoint        endTime;          /**< The NOTAM's end time */
            std::string           message;          /**< The NOTAM's message */
            std::string           rawMessage;       /**< The raw message received from the server */
            bool                  activeDueTime;    /**< The notam is active due to the activation times */

            /**
             * @brief Creates an uninitialized NOTAM
             */
            Notam() :
                    activationState(NotamActiveState::Inactive),
                    interpreterState(NotamInterpreterState::Pending),
                    title(),
                    category(NotamCategory::Unknown),
                    information(),
                    startTime(),
                    endTime(),
                    message(),
                    rawMessage(),
                    activeDueTime(false) { }

            /**
             * @brief Checks if the NOTAM is active
             * @return True if it is active, else false
             */
            bool isActive() {
                if (management::NotamActiveState::Inactive == this->activationState)
                    return false;
                else if (management::NotamActiveState::Automatic == this->activationState)
                    return this->activeDueTime;
                else
                    return true;
            }
        };

        /**
         * @brief Defines a notam that closes aerodrome sections
         */
        struct ClosedSectionNotam : public Notam {
            std::vector<std::string> sections; /**< The closed sections */
        };

        /**
         * @brief Describes closed runways
         */
        struct RunwayNotam : public ClosedSectionNotam { };

        /**
         * @brief Describes closed stands
         */
        struct StandNotam : public ClosedSectionNotam { };
    }
}

#endif
