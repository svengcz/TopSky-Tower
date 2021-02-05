/*
 * @brief Defines the boost-spirit grammar parser
 * @file grammar/Parser.hpp
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#ifndef _NDEBUG
#define BOOST_SPIRIT_DEBUG
#endif

#include <boost/fusion/adapted.hpp>
#include <boost/spirit/include/qi.hpp>

#include "AbstractSyntaxTree.hpp"

namespace topskytower {
    namespace management {
        namespace grammar {
            class Parser {
            public:
                template <typename P>
                static bool parse(const std::string& input, const P& parser, topskytower::management::grammar::AstNode& structure) {
                    auto begin(input.begin()), end(input.end());
                    return qi::phrase_parse(begin, end, parser, qi::space, structure);
                }
            };
        }
    }
}
