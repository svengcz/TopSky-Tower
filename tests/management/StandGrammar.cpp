/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the tests for the stand grammar
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <gtest/gtest.h>

#include <management/grammar/AbstractSyntaxTree.hpp>
#include <management/grammar/Stand.hpp>
#include <management/grammar/Parser.hpp>

using namespace topskytower::management::grammar;

TEST(StandGrammar, ClosedUnrestrictedAbbreviation) {
    StandGrammar grammar;
    AstNode node;

    std::string entry("ACFT STAND 61 CLSD.");

    ASSERT_TRUE(Parser::parse(entry, grammar, node));

    auto& parsed = boost::get<AstStand>(node);

    ASSERT_EQ(1UL, parsed.stands.size());
    EXPECT_EQ("61", boost::get<std::string>(parsed.stands[0]));
}

TEST(StandGrammar, ClosedUnrestrictedPlural) {
    StandGrammar grammar;
    AstNode node;

    std::string entry("ACFT STANDS 61 CLSD.");

    ASSERT_TRUE(Parser::parse(entry, grammar, node));

    auto& parsed = boost::get<AstStand>(node);

    ASSERT_EQ(1UL, parsed.stands.size());
    EXPECT_EQ("61", boost::get<std::string>(parsed.stands[0]));
}

TEST(StandGrammar, ClosedUnrestricted) {
    StandGrammar grammar;
    AstNode node;

    std::string entry("ACFT STAND 61 CLOSED.");

    ASSERT_TRUE(Parser::parse(entry, grammar, node));

    auto& parsed = boost::get<AstStand>(node);

    ASSERT_EQ(1UL, parsed.stands.size());
    EXPECT_EQ("61", boost::get<std::string>(parsed.stands[0]));
}

TEST(StandGrammar, ClosedUnrestrictedMultiple) {
    StandGrammar grammar;
    AstNode node;

    std::string entry("ACFT STANDS S418 AND S420 CLSD.");

    ASSERT_TRUE(Parser::parse(entry, grammar, node));

    auto& parsed = boost::get<AstStand>(node);

    ASSERT_EQ(2UL, parsed.stands.size());
    EXPECT_EQ("S418", boost::get<std::string>(parsed.stands[0]));
    EXPECT_EQ("S420", boost::get<std::string>(parsed.stands[1]));
}

TEST(StandGrammar, ClosedUnrestrictedMultipleExtended) {
    StandGrammar grammar;
    AstNode node;

    std::string entry("ACFT STAND 64 AND 65 CLSD.TXL N1 ABEAM ACFT STAND 64 AND 65 CLSD.");

    ASSERT_TRUE(Parser::parse(entry, grammar, node));

    auto& parsed = boost::get<AstStand>(node);

    ASSERT_EQ(2UL, parsed.stands.size());
    EXPECT_EQ("64", boost::get<std::string>(parsed.stands[0]));
    EXPECT_EQ("65", boost::get<std::string>(parsed.stands[1]));
}
