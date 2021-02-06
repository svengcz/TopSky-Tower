/*
 * @brief Defines the abstract syntax tree for the NOTAM grammatic
 * @file grammar/AstNotam.hpp
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>

#include "AbstractSyntaxTree.hpp"

namespace topskytower {
    namespace management {
        namespace grammar {
            struct AstNotamTime {
                std::string startTime;
                std::string endTime;
            };

            struct AstNotamInfo {
                std::string fir;
                std::string code;
                std::string flightRule;
                std::string purpose;
                std::string scope;
                std::string lowerAltitude;
                std::string upperAltitude;
                std::string coordinate;
                std::string radius;
            };

            struct AstNotam {
                std::string  title;
                AstNotamInfo info;
                std::string  icao;
                std::string  startTime;
                std::string  endTime;
                AstNotamTime dayTime;
                std::string  content;
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
    (std::string, lowerAltitude)
    (std::string, upperAltitude)
    (std::string, coordinate)
    (std::string, radius)
)

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstNotam,
    (std::string, title)
    (topskytower::management::grammar::AstNotamInfo, info)
    (std::string, icao)
    (std::string, startTime)
    (std::string, endTime)
    (topskytower::management::grammar::AstNotamTime, dayTime)
    (std::string, content)
)
