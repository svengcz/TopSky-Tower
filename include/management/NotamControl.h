/*
 * @brief Defines a NOTAM control system
 * @file management/NotamControl.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <functional>
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
            std::map<void*, std::function<void()>>                   m_notificationCallbacks;
            std::thread                                              m_notamThread;

            NotamControl();

            static NotamCategory parseQCode(const std::string& qCode);
            static std::shared_ptr<Notam> createNotamStructure(const std::string& qCode, const std::string& content);
            static bool createNotam(const std::string& notamText, std::shared_ptr<Notam>& notam);
            bool parseNotams(const std::string& airport);
            bool receiveNotams(const std::string& airport);
            static bool activeDueTime(const std::shared_ptr<Notam>& notam);
            void updateActiveDueTime();
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
             * @brief Returns all notams of a specific airport and a specific category
             * @note If the category is defined as NotamCategory::Unknown are all NOTAMs of an airport returned
             * @param[in] airport The airport's ICAO code
             * @param[in] category The requested category
             * @return All found NOTAMs
             */
            std::list<std::shared_ptr<Notam>> notams(const std::string& airport, NotamCategory category);
            /**
             * @brief This function has to be called, if a NOTAMs activation state is changed
             */
            void notamActivationChanged();
            /**
             * @brief Registers a callback that is triggered as soon as the NOTAMs are updated
             * @tparam T The element which registers the callback
             * @tparam F The callback function
             * @param[in] instance The instance which registers the callback
             * @param[in] cbFunction The callback function
             */
            template <typename T, typename F>
            void registerNotificationCallback(T* instance, F cbFunction) {
                std::function<void()> func = std::bind(cbFunction, instance);
                this->m_notificationCallbacks[static_cast<void*>(instance)] = func;
            }
            /**
             * @brief Deletes a callback that is triggered as soon as the NOTAMs are updated
             * @tparam T The element which registered the callback
             * @param[in] instance The instance which registers the callback
             */
            template <typename T>
            void deleteNotificationCallback(T* instance) {
                auto it = this->m_notificationCallbacks.find(static_cast<void*>(instance));
                if (this->m_notificationCallbacks.end() != it)
                    this->m_notificationCallbacks.erase(it);
            }
            /**
             * @brief Returns the NOTAM control singleton
             * @return The NOTAM control system
             */
            static NotamControl& instance();
#endif
        };
    }
}
