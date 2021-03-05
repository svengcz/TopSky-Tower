/*
 * @brief Defines the boost-spirit grammar to parse stand related NOTAMs
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

#include "AstStand.hpp"

namespace qi = boost::spirit::qi;

namespace topskytower {
    namespace management {
        namespace grammar {
            struct StandGrammar : public qi::grammar<std::string::const_iterator, AstStand()> {
                using It = std::string::const_iterator;

                qi::rule<It, AstStand()>                 root;
                qi::rule<It, std::vector<std::string>()> stands;
                qi::rule<It, std::string()>              stand;
                  
                StandGrammar() :
                        StandGrammar::base_type(this->root) {
                    this->root = qi::omit[qi::string("ACFT ")] >> qi::omit[qi::string("STANDS ") | qi::string("STAND ")] >>
                                 this->stands >> qi::omit[qi::string(" CLSD") | qi::string(" CLOSED")];

                    this->stands = *(this->stand % " AND ");
                    this->stand = +qi::char_("0-9A-Z");

#ifdef DEBUG_GRAMMAR
                    this->root.name("root");
                    this->stands.name("stands");
                    this->stand.name("stand");

                    qi::debug(this->root);
                    qi::debug(this->stands);
                    qi::debug(this->stand);
#endif
                }
            };
        }
    }
}
