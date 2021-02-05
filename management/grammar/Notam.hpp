/*
 * @brief Defines the boost-spirit grammar to parse full NOTAMs
 * @file grammar/Runway.hpp
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#define BOOST_SPIRIT_DEBUG
#pragma warning(disable: 4459)
#include <boost/config/warning_disable.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#pragma warning(default: 4459)

#include "AbstractSyntaxTree.hpp"

namespace qi = boost::spirit::qi;

namespace topskytower {
    namespace management {
        namespace grammar {
            struct NotamGrammar : public qi::grammar<std::string::const_iterator, AstNotam()> {
                using It = std::string::const_iterator;

                qi::rule<It, AstNotam()>    root;
                qi::rule<It, std::string()> title;
                qi::rule<It, std::string()> qEntry;
                qi::rule<It, std::string()> icaoEntry;
                qi::rule<It, std::string()> dateEntry;
                qi::rule<It, std::string()> notamEntry;

                NotamGrammar() :
                        NotamGrammar::base_type(this->root) {
                    this->root = this->title >> this->qEntry >> this->icaoEntry >> this->dateEntry >> this->dateEntry >> this->notamEntry;

                    /* define the title */
                    this->title = +qi::char_("a-zA-Z0-9/ ") >> qi::eol;

                    /* define the single entries that are seperated by ')' */
                    this->qEntry = qi::char_('Q') >> ") " >> +qi::char_("a-zA-Z0-9/") >> qi::omit[qi::char_(' ') | qi::char_('\n')];
                    this->icaoEntry = "A) " >> qi::repeat(4)[qi::char_("A-Z")] >> qi::omit[qi::char_(' ') | qi::char_('\n')];
                    this->dateEntry = qi::omit[qi::char_("BC")] >> ") " >> qi::repeat(10)[qi::char_("0-9")] >> qi::omit[qi::char_(' ') | qi::char_('\n')];
                    this->notamEntry = "E) " >> +(qi::char_ - "CREATED : ");
                }
            };
        }
    }
}

template <typename P>
bool parse(const std::string& input, const P& parser, topskytower::management::grammar::AstNode& structure) {
    auto begin(input.begin()), end(input.end());
    bool retval = qi::phrase_parse(begin, end, parser, qi::space, structure);

    topskytower::management::grammar::AstDebug debug(structure);
    std::string value = debug.stream.str();

    if (false == retval || begin != end) {
        value += "\n\nRemaining:\n";
        value += std::string(begin, end);
    }

    return retval;
}
