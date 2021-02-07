/*
 * @brief Defines the abstract syntax tree for the runway grammar
 * @file grammar/AstRunway.hpp
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>
#include <utility>

#include <types/Quantity.hpp>

#include "AstBase.hpp"

namespace topskytower {
    namespace management {
        namespace grammar {
            struct AstRunwayDimension : public topskytower::types::Length { };

            struct AstRunway {
                AstNodeVector      names;
            };
        }
    }
}

BOOST_FUSION_ADAPT_STRUCT(
    topskytower::management::grammar::AstRunway,
    (topskytower::management::grammar::AstNodeVector, names)
);
