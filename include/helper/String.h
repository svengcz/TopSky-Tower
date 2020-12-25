/*
 * @brief Defines and implements functions to handle strings
 * @file helper/String.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>
#include <vector>

namespace topskytower {
    namespace helper {
        /**
         * @brief Implements and defines convinience functions for the string handling
         * @ingroup helper
         */
        class String {
        private:
            template <typename Separator>
            static auto splitAux(const std::string& value, Separator&& separator) -> std::vector<std::string> {
                std::vector<std::string> result;
                std::string::size_type p = 0;
                std::string::size_type q;
                while ((q = separator(value, p)) != std::string::npos) {
                    result.emplace_back(value, p, q - p);
                    p = q + 1;
                }
                result.emplace_back(value, p);
                return result;
            }

        public:
            String() = delete;
            String(const String&) = delete;
            String(String&&) = delete;
            String& operator=(const String&) = delete;
            String& operator=(String&&) = delete;

            /**
             * @brief Replaces all markers by replace in message
             * @param[in,out] message The message which needs to be modified
             * @param[in] marker The wildcard which needs to be found in message and which needs to be replaced
             * @param[in] replace The replacement of marker in message
             * @return
             */
            static __inline void stringReplace(std::string& message, const std::string& marker, const std::string& replace) {
                std::size_t pos = message.find(marker, 0);
                while (std::string::npos != pos) {
                    auto it = message.cbegin() + pos;
                    message.replace(it, it + marker.length(), replace);
                    pos = message.find(marker, pos + marker.length());
                }
            }

            /**
             * @brief Splits value into chunks and the separator is defined in separators
             * @param[in] value The string which needs to be splitted up
             * @param[in] separators The separators which split up the value
             * @return The list of splitted chunks
             */
            static auto splitString(const std::string& value, const std::string& separators) -> std::vector<std::string> {
                return String::splitAux(value, [&](const std::string& v, std::string::size_type p) {
                                                   return v.find_first_of(separators, p);
                                               });
            }
        };
    }
}
