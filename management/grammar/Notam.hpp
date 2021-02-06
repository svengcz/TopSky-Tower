/*
 * @brief Defines the boost-spirit grammar to parse full NOTAMs
 * @file grammar/Runway.hpp
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

namespace topskytower {
    namespace management {
        namespace grammar {
            struct NotamGrammar : public qi::grammar<std::string::const_iterator, AstNotam()> {
                using It = std::string::const_iterator;

                qi::rule<It, AstNotam()>     root;
                qi::rule<It, std::string()>  title;
                qi::rule<It, AstNotamInfo()> qEntry;
                qi::rule<It, std::string()>  icaoEntry;
                qi::rule<It, std::string()>  startEntry;
                qi::rule<It, std::string()>  endEntry;
                qi::rule<It, AstNotamTime()> dayTimeEntry;
                qi::rule<It, std::string()>  notamEntry;
                qi::rule<It, std::string()>  clockDefinition;
                qi::rule<It, std::string()>  endOfEntry;
                qi::rule<It, std::string()>  dateDefinition;
                qi::rule<It, std::string()>  firEntry;
                qi::rule<It, std::string()>  qCode;
                qi::rule<It, std::string()>  flightRule;
                qi::rule<It, std::string()>  purpose;
                qi::rule<It, std::string()>  scope;
                qi::rule<It, std::string()>  altitude;
                qi::rule<It, std::string()>  coordinate;
                qi::rule<It, std::string()>  radius;

                NotamGrammar() :
                        NotamGrammar::base_type(this->root) {
                    this->root = this->title >> this->qEntry >> this->icaoEntry >> this->startEntry >> this->endEntry >> -this->dayTimeEntry >> this->notamEntry;

                    /* define the title */
                    this->title = +qi::char_("a-zA-Z0-9/ ") >> qi::eol;

                    /* define the single entries that are seperated by ')' */
                    this->qEntry = "Q) " >> this->firEntry >> '/' >> this->qCode >> '/' >> this->flightRule >> '/' >> this->purpose >> '/' >> this->scope >> '/' >>
                                   this->altitude >> '/' >> this->altitude >> '/' >> this->coordinate >> this->radius >> qi::omit[this->endOfEntry];
                    this->icaoEntry = "A) " >> qi::repeat(4)[qi::char_("A-Z")] >> qi::omit[this->endOfEntry];
                    this->startEntry = "B) " >> this->dateDefinition >> qi::omit[this->endOfEntry];
                    this->endEntry = "C) " >> (this->dateDefinition | "PERM") >> -qi::no_case[" EST"] >> qi::omit[this->endOfEntry];
                    this->dayTimeEntry = "D) " >> -qi::omit["DAILY "] >> this->clockDefinition >> "-" >> this->clockDefinition >> qi::omit[this->endOfEntry];
                    this->notamEntry = "E) " >> +(qi::char_ - "CREATED : ");

                    /* define some generic entries */
                    this->clockDefinition = qi::repeat(4)[qi::char_("0-9")];
                    this->endOfEntry = qi::char_(' ') | qi::char_('\n');
                    this->dateDefinition = qi::repeat(10)[qi::char_("0-9")];

                    /* some Q-code definitions */
                    this->firEntry = qi::repeat(4)[qi::char_("A-Z")];
                    this->qCode = qi::repeat(5)[qi::char_("A-Z")];
                    this->flightRule = +qi::char_("IV ") | qi::no_case["IV"];
                    this->purpose = +qi::char_("NBOM ");
                    this->scope = +qi::char_("AEW ");
                    this->altitude = qi::repeat(3)[qi::char_("0-9")];
                    this->coordinate = qi::repeat(4)[qi::char_("0-9")] >> qi::char_("NS") >> qi::repeat(5)[qi::char_("0-9")] >> qi::char_("WE");
                    this->radius = -qi::repeat(3)[qi::char_("0-9")];
                }
            };
        }
    }
}
