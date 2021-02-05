/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the tests for the flight registry
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <gtest/gtest.h>

#include <management/grammar/Notam.hpp>
#include <management/grammar/Parser.hpp>

using namespace topskytower;

TEST(Grammar, NOTAM) {
    topskytower::management::grammar::NotamGrammar grammar;
    topskytower::management::grammar::AstNode node;

    std::string notam = "W0095/21 NOTAMN\n\
Q) EDWW/QPIGV/IV/NBO/A/000/999/5221N01330E005 A) EDDB B) 2101281521 C) 2102250001\n\
E) AERODROME[US DOD E - IPL ONLY] VOR RWY 25L WITH JULIAN DATE OF\n\
20358 NOT AUTHORIZED\n\
CREATED : 28 Jan 2021 15 : 21 : 00\n\
SOURCE : KQZC";

    EXPECT_TRUE(management::grammar::Parser::parse(notam, grammar, node));
}

