/*
 * @brief Defines the abstract syntax tree
 * @file grammar/AbstractSyntaxTree.hpp
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <vector>

#pragma warning(disable: 4459)
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#pragma warning(default: 4459)

namespace topskytower {
    namespace management {
        namespace grammar {
            struct AstNull;
            struct AstNodeVector;
            struct AstString;
            struct AstInteger;
            struct AstFloat;
            struct AstKeyValue;
            struct AstNotamTime;
            struct AstNotamInfo;
            struct AstNotam;

            /**< Defines the generic node structure */
            typedef boost::variant <
                AstNull,
                AstString,
                AstInteger,
                AstFloat,
                boost::recursive_wrapper<AstNodeVector>,
                AstKeyValue,
                AstNotamTime,
                AstNotamInfo,
                AstNotam
            > AstNode;

            struct AstNull { };

            struct AstNodeVector : public std::vector<AstNode> { };

            struct AstString : public std::string { };

            struct AstInteger {
                std::int64_t value;
                explicit inline AstInteger(const std::int64_t& v = 0) : value(v) { }
            };

            struct AstFloat {
                float value;
                explicit inline AstFloat(const float& v = 0.0f) : value(v) { }
            };

            struct AstKeyValue {
                std::string key;
                std::string value;
            };

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
    topskytower::management::grammar::AstInteger,
    (std::int64_t, value)
);

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstFloat,
    (float, value)
);

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstKeyValue,
    (std::string, key)
    (std::string, value)
)

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
