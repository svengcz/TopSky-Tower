/*
 * @brief Defines the abstract syntax tree for the runway grammar
 * @file grammar/AstRunway.hpp
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace topskytower {
    namespace management {
        namespace grammar {
            struct AstRunway {
                std::vector<std::string> names;
            };
        }
    }
}

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstRunway,
    (std::vector<std::string>, names)
);
