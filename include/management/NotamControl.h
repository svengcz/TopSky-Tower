/*
 * @brief Defines a NOTAM control system
 * @file management/NotamControl.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <thread>
#include <list>
#include <map>
#include <mutex>

#include <types/Coordinate.h>
#include <types/SystemConfiguration.h>

namespace topskytower {
    namespace management {
        /**
         * @brief The NOTAM control handles all available and relevant NOTAMs of the airports
         * @ingroup management
         *
         * The NOTAM control contains all NOTAMs of the controlled aerodromes.
         * It provides a table that contains all downloaded NOTAMs.
         * The table can be filtered by the aerodrome's ICAO code or by the checkbox that
         * filters out all NOTAMs that are currently invalid due to the current time.
         *
         * ![NOTAM table](doc/imgs/NotamOverview.png)
         *
         * A click on a specific NOTAM opens the entry and visualizes a second window that
         * shows the content of the specific NOTAM.
         *
         * ![NOTAM details](doc/imgs/NotamReader.png)
         */
        class NotamControl {
        public:
#ifndef DOXYGEN_IGNORE
            using TimePoint = std::chrono::system_clock::time_point; /**< Defines a more readable type for the time_point */

            /**
             * @brief Defines the NOTAM categories based on the FAA releases
             */
            enum class Category {
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
                std::string      title;       /**< The NOTAM's title */
                Category         category;    /**< The NOTAM's category */
                NotamInformation information; /**< The NOTAM's information */
                TimePoint        startTime;   /**< The NOTAM's start time */
                TimePoint        endTime;     /**< The NOTAM's end time */
                std::string      message;     /**< The NOTAM's message */
                std::string      rawMessage;  /**< The raw message received from the server */

                Notam() :
                        title(),
                        category(Category::Unknown),
                        information(),
                        startTime(),
                        endTime(),
                        message(),
                        rawMessage() { }
            };

        private:
            volatile bool                           m_stopNotamThread;
            std::map<std::string, TimePoint>        m_airportUpdates;
            std::mutex                              m_pendingQueueLock;
            std::list<std::string>                  m_enqueuePending;
            std::list<std::string>                  m_dequeuePending;
            std::map<std::string, std::list<Notam>> m_notams;
            std::thread                             m_notamThread;

            NotamControl();

            static Category parseQCode(const std::string& qCode);
            static bool createNotam(const std::string& notamText, NotamControl::Notam& notam);
            bool parseNotams(const std::string& airport);
            bool receiveNotams(const std::string& airport);
            void run();

        public:
            /**
             * @brief Destroys all internal structures
             */
            ~NotamControl();

            NotamControl(const NotamControl& other) = delete;
            NotamControl(NotamControl&& other) = delete;

            NotamControl& operator=(const NotamControl& other) = delete;
            NotamControl& operator=(NotamControl&& other) = delete;

            /**
             * @brief Adds a new airport to the NOTAM control system
             * @param[in] airport The airport's ICAO code
             */
            void addAirport(const std::string& airport);
            /**
             * @brief Removes an airport out of the NOTAM control system
             * @param[in] airport The airport's ICAO code
             */
            void removeAirport(const std::string& airport);
            /**
             * @brief Returns all received NOTAMs
             * @return The NOTAMs
             */
            const std::map<std::string, std::list<Notam>>& notams() const;
            /**
             * @brief Returns the NOTAM control singleton
             * @return The NOTAM control system
             */
            static NotamControl& instance();
#endif
        };
    }
}
