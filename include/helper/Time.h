/*
 * @brief Defines and implements functions to handle timestamps
 * @file helper/Time.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <ctime>
#include <chrono>
#include <sstream>
#include <string>
#include <iomanip>

namespace topskytower {
    namespace helper {
        /**
         * @brief Defines helper functions to handle system timestamps and UTC timestamps
         * @ingroup helper
         */
        class Time {
        public:
            Time() = delete;
            Time(const Time&) = delete;
            Time(Time&&) = delete;
            Time& operator=(const Time&) = delete;
            Time& operator=(Time&&) = delete;

            /**
             * @brief Returns the current time in UTC
             * @return The current UTC time
             */
            static __inline std::chrono::system_clock::time_point currentUtc() {
                std::time_t now = std::time(0);
                std::tm* nowTm = std::gmtime(&now);
                return std::chrono::system_clock::from_time_t(std::mktime(nowTm));
            }

            /**
             * @brief Converts a string into a std::chrono::system_clock::time_point in UTC
             * The function assumes the following format:
             * - %y%m%d%H%M
             * @param[in] date The string containing the timepoint
             * @return The converted time
             */
            static __inline std::chrono::system_clock::time_point stringToTime(const std::string& date) {
                if (10 > date.length())
                    return std::chrono::system_clock::time_point();

                /* convert the string to a string with spaces due to an MSVC-runtime bug */
                std::string convertedDate;
                for (int i = 0; i < 10; i += 2)
                    convertedDate += date.substr(i, 2) + " ";
                std::istringstream stream(convertedDate);

                struct std::tm tm;
                stream >> std::get_time(&tm, "%y %m %d %H %M");
                tm.tm_wday = tm.tm_yday = tm.tm_sec = 0;
                tm.tm_isdst = -1;

                /* estimate the local time offset */
                static const std::time_t epochTest = 60 * 60 * 11;
                static auto localHours = std::localtime(&epochTest)->tm_hour;
                static auto utcHours = std::gmtime(&epochTest)->tm_hour;
                static auto diffHours = localHours - utcHours;
                tm.tm_hour += diffHours;

                return std::chrono::system_clock::from_time_t(std::mktime(&tm));
            }

            /**
             * @brief Converts a time point into a string
             * The function converts into the following format:
             * - %y%m%d%H%M
             * @param[in] time The timepoint which needs to be converted in UTC
             * @return The converted time
             */
            static __inline std::string timeToString(const std::chrono::system_clock::time_point& time, const std::string& format = "%y%m%d%H%M") {
                if ((std::chrono::time_point<std::chrono::system_clock>::max)() != time) {
                    std::time_t value = std::chrono::system_clock::to_time_t(time);
                    std::stringstream stream;
                    stream << std::put_time(std::gmtime(&value), format.c_str());
                    return stream.str();
                }
                else {
                    return "Permanent";
                }
            }
        };
    }
}
