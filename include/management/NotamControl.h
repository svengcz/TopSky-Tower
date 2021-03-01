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
#include <memory>
#include <mutex>

#include <management/Notam.h>
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
        private:
            volatile bool                                            m_stopNotamThread;
            std::map<std::string, NotamTimePoint>                    m_airportUpdates;
            std::mutex                                               m_pendingQueueLock;
            std::list<std::string>                                   m_enqueuePending;
            std::list<std::string>                                   m_dequeuePending;
            std::map<std::string, std::list<std::shared_ptr<Notam>>> m_notams;
            std::thread                                              m_notamThread;

            NotamControl();

            static NotamCategory parseQCode(const std::string& qCode);
            static std::shared_ptr<Notam> createNotamStructure(const std::string& qCode, const std::string& content);
            static bool createNotam(const std::string& notamText, std::shared_ptr<Notam>& notam);
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
            const std::map<std::string, std::list<std::shared_ptr<Notam>>>& notams() const;
            /**
             * @brief Returns the NOTAM control singleton
             * @return The NOTAM control system
             */
            static NotamControl& instance();
#endif
        };
    }
}
