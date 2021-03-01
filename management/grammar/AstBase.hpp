/*
 * @brief Defines the abstract syntax tree
 * @file grammar/AstBase.hpp
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
            struct AstNotam;
            struct AstRunway;

            /**< Defines the generic node structure */
            typedef boost::variant <
                AstNull,
                std::string,
                AstNotam,
                AstRunway
            > AstNode;

            struct AstNull { };
        }
    }
}
