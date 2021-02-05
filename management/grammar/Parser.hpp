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
                    bool retval = qi::phrase_parse(begin, end, parser, qi::space, structure);

#ifndef NDEBUG
                    topskytower::management::grammar::AstDebug debug(structure);
                    std::string value = debug.stream.str();

                    if (false == retval || begin != end) {
                        value += "\n\nRemaining:\n";
                        value += std::string(begin, end);
                    }
#endif

                    return retval;
                }
            };
        }
    }
}
