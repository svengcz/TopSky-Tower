/*
 * @brief Defines a NOTAM control system
 * @file management/NotamControl.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <thread>
#include <list>
#include <map>
#include <mutex>

#include <types/SystemConfiguration.h>

namespace topskytower {
    namespace management {
        /**
         * @brief The NOTAM control handles all available and relevant NOTAMs of the airports
         * @ingroup management
         */
        class NotamControl {
        public:
#ifndef DOXYGEN_IGNORE
            using TimePoint = std::chrono::system_clock::time_point; /**< Defines a more readable type for the time_point */

            /**
             * @brief Defines a parsed NOTAM
             */
            struct Notam {
                std::string title;      /**< The NOTAM's title */
                TimePoint   startTime;  /**< The NOTAM's start time */
                TimePoint   endTime;    /**< The NOTAM's end time */
                std::string message;    /**< The NOTAM's message */
                std::string rawMessage; /**< The raw message received from the server */
            };

        private:
            volatile bool                           m_stopNotamThread;
            std::thread                             m_notamThread;
            std::map<std::string, TimePoint>        m_airportUpdates;
            std::mutex                              m_pendingQueueLock;
            std::list<std::string>                  m_enqueuePending;
            std::list<std::string>                  m_dequeuePending;
            std::map<std::string, std::list<Notam>> m_notams;

            NotamControl();

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