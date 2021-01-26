/*
 * @brief Defines and implements functions to handle timestamps
 * @file helper/Time.h
 * @author Sven Czarnian <devel@svcz.de>
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
                return std::chrono::system_clock::from_time_t(std::mktime(&tm));
            }

            /**
             * @brief Converts a time point into a string
             * The function converts into the following format:
             * - %y%m%d%H%M
             * @param[in] time The timepoint which needs to be converted in UTC
             * @return The converted time
             */
            static __inline std::string timeToString(const std::chrono::system_clock::time_point& time) {
                char buf[11];

                std::time_t value = std::chrono::system_clock::to_time_t(time);
                std::tm local = *std::localtime(&value);

                std::strftime(buf, 11, "%y%m%d%H%M", &local);

                return std::string(buf);
            }
        };
    }
}
