/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the tests for the runway grammar
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <gtest/gtest.h>

#include <management/grammar/AbstractSyntaxTree.hpp>
#include <management/grammar/Parser.hpp>
#include <management/grammar/Runway.hpp>

using namespace topskytower::management::grammar;
using namespace topskytower::types;

TEST(RunwayGrammar, ClosedUnrestrictedAbbreviation) {
    RunwayGrammar grammar;
    AstNode node;

    std::string entry("RWY 07R/25L CLSD.");

    ASSERT_TRUE(Parser::parse(entry, grammar, node));

    auto& parsed = boost::get<AstRunway>(node);

    ASSERT_EQ(2UL, parsed.names.size());
    EXPECT_EQ("07R", boost::get<std::string>(parsed.names[0]));
    EXPECT_EQ("25L", boost::get<std::string>(parsed.names[1]));
}

TEST(RunwayGrammar, ClosedUnrestricted) {
    RunwayGrammar grammar;
    AstNode node;

    std::string entry("RWY 07R/25L CLOSED.");

    ASSERT_TRUE(Parser::parse(entry, grammar, node));

    auto& parsed = boost::get<AstRunway>(node);

    ASSERT_EQ(2UL, parsed.names.size());
    EXPECT_EQ("07R", boost::get<std::string>(parsed.names[0]));
    EXPECT_EQ("25L", boost::get<std::string>(parsed.names[1]));
}

TEST(RunwayGrammar, ClosedUnrestrictedExplanation) {
    RunwayGrammar grammar;
    AstNode node;

    std::string entry("RWY 05L/23R CLSD, DUE TO REDUCED FIRE BRIGADE COVERAGE.");

    ASSERT_TRUE(Parser::parse(entry, grammar, node));

    auto& parsed = boost::get<AstRunway>(node);

    ASSERT_EQ(2UL, parsed.names.size());
    EXPECT_EQ("05L", boost::get<std::string>(parsed.names[0]));
    EXPECT_EQ("23R", boost::get<std::string>(parsed.names[1]));
}

TEST(RunwayGrammar, ClosedUnrestrictedMultiple) {
    RunwayGrammar grammar;
    AstNode node;

    std::string entry("RWY 09R/27L AND 09L/27R CLOSED");

    ASSERT_TRUE(Parser::parse(entry, grammar, node));

    auto& parsed = boost::get<AstRunway>(node);

    ASSERT_EQ(4UL, parsed.names.size());
    EXPECT_EQ("09R", boost::get<std::string>(parsed.names[0]));
    EXPECT_EQ("27L", boost::get<std::string>(parsed.names[1]));
    EXPECT_EQ("09L", boost::get<std::string>(parsed.names[2]));
    EXPECT_EQ("27R", boost::get<std::string>(parsed.names[3]));
}
