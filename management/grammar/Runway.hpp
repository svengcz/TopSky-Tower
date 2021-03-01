/*
 * @brief Defines the boost-spirit grammar to parse runway related NOTAMs
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

#include "AstRunway.hpp"

namespace qi = boost::spirit::qi;

namespace topskytower {
    namespace management {
        namespace grammar {
            struct RunwayGrammar : public qi::grammar<std::string::const_iterator, AstRunway(), qi::space_type> {
                using It = std::string::const_iterator;

                qi::rule<It, AstRunway(), qi::space_type>                root;
                qi::rule<It, std::vector<std::string>(), qi::space_type> runways;
                qi::rule<It, std::vector<std::string>(), qi::space_type> runwayNames;
                qi::rule<It, std::string(), qi::space_type>              runway;

                RunwayGrammar() :
                        RunwayGrammar::base_type(this->root) {
                    this->root = "RWY" >> this->runways >> qi::omit[qi::string("CLSD") | qi::string("CLOSED")];

                    this->runways = *(this->runwayNames % "AND");
                    this->runwayNames = this->runway >> '/' >> this->runway;
                    this->runway = qi::repeat(2)[qi::char_("0-9")] >> -qi::char_("LRC");

#ifdef DEBUG_GRAMMAR
                    root.name("root");
                    runways.name("runways");
                    runwayNames.name("names");
                    runway.name("runway");

                    qi::debug(this->root);
                    qi::debug(this->runways);
                    qi::debug(this->runwayNames);
                    qi::debug(this->runway);
#endif
                }
            };
        }
    }
}
