/*
 * @brief Defines the abstract syntax tree for the stand grammar
 * @file grammar/AstStand.hpp
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace topskytower {
    namespace management {
        namespace grammar {
            struct AstStand {
                std::vector<std::string> stands;
            };
        }
    }
}

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstStand,
    (std::vector<std::string>, stands)
);
