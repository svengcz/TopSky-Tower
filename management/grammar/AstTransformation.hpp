/*
 * @brief Defines the boost-spirit transformation classes to transform files
 * @file grammar/AstTransformation.hpp
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#pragma warning(disable: 4459)
#include <boost/config/warning_disable.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#pragma warning(default: 4459)

#include "AstNotam.hpp"

namespace qi = boost::spirit::qi;

namespace boost {
    namespace spirit {
        namespace traits {
            template<>
            struct transform_attribute<std::chrono::system_clock::time_point, std::string, qi::domain> {
                typedef std::string type;

                static std::string pre(std::chrono::system_clock::time_point) {
                    return std::string();
                }

                static void post(std::chrono::system_clock::time_point& timestamp, std::string const& stamp) {
                    if (0 != stamp.length())
                        timestamp = topskytower::helper::Time::stringToTime(stamp);
                }

                static void fail(std::chrono::system_clock::time_point&) { }
            };

            template<>
            struct transform_attribute<topskytower::management::grammar::AstNotamAltitude, int, qi::domain> {
                typedef int type;

                static int pre(topskytower::management::grammar::AstNotamAltitude) {
                    return int();
                }

                static void post(topskytower::management::grammar::AstNotamAltitude& length, int const& value) {
                    static_cast<topskytower::types::Length&>(length) = static_cast<float>(value * 100) * topskytower::types::feet;
                }

                static void fail(topskytower::management::grammar::AstNotamAltitude&) { }
            };

            template<>
            struct transform_attribute<topskytower::management::grammar::AstNotamRadius, int, qi::domain> {
                typedef int type;

                static int pre(topskytower::management::grammar::AstNotamRadius) {
                    return int();
                }

                static void post(topskytower::management::grammar::AstNotamRadius& length, int const& value) {
                    static_cast<topskytower::types::Length&>(length) = static_cast<float>(value) * topskytower::types::nauticmile;
                }

                static void fail(topskytower::management::grammar::AstNotamRadius&) { }
            };

            template<>
            struct transform_attribute<topskytower::types::Coordinate, std::string, qi::domain> {
                typedef std::string type;

                static std::string pre(topskytower::types::Coordinate) {
                    return std::string();
                }

                static void post(topskytower::types::Coordinate& coordinate, std::string const& value) {
                    std::string latitude = std::string(1, value[4]) + "0" + value.substr(0, 2) + "." + value.substr(2, 2) + ".00.000";
                    std::string longitude = value[10] + value.substr(5, 3) + "." + value.substr(8, 2) + ".00.000";
                    coordinate = topskytower::types::Coordinate(longitude, latitude);
                }

                static void fail(topskytower::types::Coordinate&) { }
            };
        }
    }
}
