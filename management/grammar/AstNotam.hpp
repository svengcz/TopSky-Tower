/*
 * @brief Defines the abstract syntax tree for the NOTAM grammar
 * @file grammar/AstNotam.hpp
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>

#include <helper/Time.h>
#include <types/Coordinate.h>

#include "AstBase.hpp"

namespace topskytower {
    namespace management {
        namespace grammar {
            struct AstNotamTime {
                std::string startTime;
                std::string endTime;
            };

            struct AstNotamAltitude : public types::Length { };

            struct AstNotamRadius : public types::Length { };

            struct AstNotamInfo {
                std::string       fir;
                std::string       code;
                std::string       flightRule;
                std::string       purpose;
                std::string       scope;
                AstNotamAltitude  lowerAltitude;
                AstNotamAltitude  upperAltitude;
                types::Coordinate coordinate;
                AstNotamRadius    radius;
            };

            struct AstNotam {
                std::string                           title;
                AstNotamInfo                          info;
                std::string                           icao;
                std::chrono::system_clock::time_point startTime;
                std::chrono::system_clock::time_point endTime;
                AstNotamTime                          dayTime;
                std::string                           content;

                AstNotam() :
                        title(),
                        info(),
                        icao(),
                        startTime((std::chrono::time_point<std::chrono::system_clock>::min)()),
                        endTime((std::chrono::time_point<std::chrono::system_clock>::max)()),
                        dayTime(),
                        content() { }
            };
        }
    }
}

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstNotamTime,
    (std::string, startTime)
    (std::string, endTime)
)

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstNotamInfo,
    (std::string, fir)
    (std::string, code)
    (std::string, flightRule)
    (std::string, purpose)
    (std::string, scope)
    (topskytower::management::grammar::AstNotamAltitude, lowerAltitude)
    (topskytower::management::grammar::AstNotamAltitude, upperAltitude)
    (topskytower::types::Coordinate, coordinate)
    (topskytower::management::grammar::AstNotamRadius, radius)
)

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstNotam,
    (std::string, title)
    (topskytower::management::grammar::AstNotamInfo, info)
    (std::string, icao)
    (std::chrono::system_clock::time_point, startTime)
    (std::chrono::system_clock::time_point, endTime)
    (topskytower::management::grammar::AstNotamTime, dayTime)
    (std::string, content)
)
