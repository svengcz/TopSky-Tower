/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the tests for the NOTAM grammatic
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <gtest/gtest.h>

#include <management/grammar/Notam.hpp>
#include <management/grammar/Parser.hpp>

using namespace topskytower::management::grammar;

TEST(NotamGrammar, NotamDayTime) {
    NotamGrammar grammar;
    AstNode node;

    std::string content = "DUE TO MAINT OF LWB DVORDME THE FLW CHANGES TO APCH PROC OCCUR:\n\
RWY 07L ILS/LOC AND RWY 07R ILS/LOC/VOR: STANDARD APCH FROM\n\
KETAP AND OGBER TEMP SUSPENDED. EXPECT VECTORING TO FINAL.\n\
REF AD 2 EDDB 4-2-1/-2 DATED 28 JAN 2021 AND 4-3-1 ALL DATED 03 DEC \n\
2020.\n\
RWY 25L ILS/LOC/VOR AND RWY 25R ILS/LOC: STANDARD APCH FROM\n\
OGBER TEMP SUSPENDED. EXPECT VECTORING TO FINAL.\n\
REF AD 2 EDDB 4-2-3/-4 DATED 28 JAN 2021.";
    std::string notam("A0625/21 NOTAMN\n\
Q) EDWW/QPICH/I /NBO/A /000/999/5222N01330E005\n\
A) EDDB B) 2102160700 C) 2102181500\n\
D) 0700-1500\n\
E) " + content);

    ASSERT_TRUE(Parser::parse(notam, grammar, node));

    auto& parsed = boost::get<AstNotam>(node);
    EXPECT_EQ("A0625/21 NOTAMN", parsed.title);
    EXPECT_EQ("EDWW", parsed.info.fir);
    EXPECT_EQ("QPICH", parsed.info.code);
    EXPECT_EQ("I ", parsed.info.flightRule);
    EXPECT_EQ("NBO", parsed.info.purpose);
    EXPECT_EQ("A ", parsed.info.scope);
    EXPECT_EQ("000", parsed.info.lowerAltitude);
    EXPECT_EQ("999", parsed.info.upperAltitude);
    EXPECT_EQ("5222N01330E", parsed.info.coordinate);
    EXPECT_EQ("005", parsed.info.radius);
    EXPECT_EQ("EDDB", parsed.icao);
    EXPECT_EQ("2102160700", parsed.startTime);
    EXPECT_EQ("2102181500", parsed.endTime);
    EXPECT_EQ("0700", parsed.dayTime.startTime);
    EXPECT_EQ("1500", parsed.dayTime.endTime);
    EXPECT_EQ(content, parsed.content);
}

TEST(NotamGrammar, NotamPerm) {
    NotamGrammar grammar;
    AstNode node;

    std::string content = "LOC APCH 25L:\n\
AMEND MAPT 2.6 DME BBI TO READ 3.2 DME BBI.\n\
AMEND MATF 2.7 DME SDD TO READ 2.2 DME SDD.\n\
REF AD 2 EDDB 4-2-3 DATED 28 JAN 2021.\n\
VOR APCH 25L:\n\
AMEND MATF 2.7 DME SDD TO READ 2.2 DME SDD.\n\
REF AD 2 EDDB 4-3-2 DATED 28 JAN 2021.\n\
RNP APCH RWY 25L:\n\
AMEND TRACK FROM OGBER TO DB853 TO READ MAG 095 (TRUE 098.4) DEG.\n\
REF AD 2 EDDB 4-6-7 DATED 31 DEZ 2020, 4-6-8 EFF 04 NOV 2020.";
    std::string notam("A0596/21 NOTAMN\n\
Q) EDWW/QPICH/I /NBO/A /000/999/5222N01330E005\n\
A) EDDB B) 2102011423 C) PERM\n\
E) " + content);

    ASSERT_TRUE(Parser::parse(notam, grammar, node));

    auto& parsed = boost::get<AstNotam>(node);
    EXPECT_EQ("A0596/21 NOTAMN", parsed.title);
    EXPECT_EQ("EDWW", parsed.info.fir);
    EXPECT_EQ("QPICH", parsed.info.code);
    EXPECT_EQ("I ", parsed.info.flightRule);
    EXPECT_EQ("NBO", parsed.info.purpose);
    EXPECT_EQ("A ", parsed.info.scope);
    EXPECT_EQ("000", parsed.info.lowerAltitude);
    EXPECT_EQ("999", parsed.info.upperAltitude);
    EXPECT_EQ("5222N01330E", parsed.info.coordinate);
    EXPECT_EQ("005", parsed.info.radius);
    EXPECT_EQ("EDDB", parsed.icao);
    EXPECT_EQ("2102011423", parsed.startTime);
    EXPECT_EQ(0UL, parsed.endTime.length());
    EXPECT_EQ(0UL, parsed.dayTime.startTime.length());
    EXPECT_EQ(0UL, parsed.dayTime.endTime.length());
    EXPECT_EQ(content, parsed.content);
}

TEST(NotamGrammar, NotamEndEstimated) {
    NotamGrammar grammar;
    AstNode node;

    std::string content = "ALL RELATED IFR PROC TEMPO SUSPENDED, DUE TO RWY 07R/25L CLSD.";
    std::string notam("A0569/21 NOTAMR A8000/20\n\
Q) EDWW/QPIAU/I /NBO/A /000/999/5222N01330E005\n\
A) EDDB B) 2102010805 C) 2105010800 EST\n\
E) " + content);

    ASSERT_TRUE(Parser::parse(notam, grammar, node));

    auto& parsed = boost::get<AstNotam>(node);
    EXPECT_EQ("A0569/21 NOTAMR A8000/20", parsed.title);
    EXPECT_EQ("EDWW", parsed.info.fir);
    EXPECT_EQ("QPIAU", parsed.info.code);
    EXPECT_EQ("I ", parsed.info.flightRule);
    EXPECT_EQ("NBO", parsed.info.purpose);
    EXPECT_EQ("A ", parsed.info.scope);
    EXPECT_EQ("000", parsed.info.lowerAltitude);
    EXPECT_EQ("999", parsed.info.upperAltitude);
    EXPECT_EQ("5222N01330E", parsed.info.coordinate);
    EXPECT_EQ("005", parsed.info.radius);
    EXPECT_EQ("EDDB", parsed.icao);
    EXPECT_EQ("2102010805", parsed.startTime);
    EXPECT_EQ("2105010800", parsed.endTime);
    EXPECT_EQ(0UL, parsed.dayTime.startTime.length());
    EXPECT_EQ(0UL, parsed.dayTime.endTime.length());
    EXPECT_EQ(content, parsed.content);
}

TEST(NotamGrammar, NotamDaily) {
    NotamGrammar grammar;
    AstNode node;

    std::string content = "VOR/DME FWE/FUERSTENWALDE 113.3MHZ/CH80X ON MAINT. DO NOT USE.\n\
DISREGARD ALL SIGNALS.";
    std::string notam("A0430/21 NOTAMN\n\
Q) EDWW/QNMCT/IV/BO /AE/000/999/5225N01408E025\n\
A) EDDB B) 2102230800 C) 2102241400\n\
D) DAILY 0800-1400\n\
E) " + content);

    ASSERT_TRUE(Parser::parse(notam, grammar, node));

    auto& parsed = boost::get<AstNotam>(node);
    EXPECT_EQ("A0430/21 NOTAMN", parsed.title);
    EXPECT_EQ("EDWW", parsed.info.fir);
    EXPECT_EQ("QNMCT", parsed.info.code);
    EXPECT_EQ("IV", parsed.info.flightRule);
    EXPECT_EQ("BO ", parsed.info.purpose);
    EXPECT_EQ("AE", parsed.info.scope);
    EXPECT_EQ("000", parsed.info.lowerAltitude);
    EXPECT_EQ("999", parsed.info.upperAltitude);
    EXPECT_EQ("5225N01408E", parsed.info.coordinate);
    EXPECT_EQ("025", parsed.info.radius);
    EXPECT_EQ("EDDB", parsed.icao);
    EXPECT_EQ("2102230800", parsed.startTime);
    EXPECT_EQ("2102241400", parsed.endTime);
    EXPECT_EQ("0800", parsed.dayTime.startTime);
    EXPECT_EQ("1400", parsed.dayTime.endTime);
    EXPECT_EQ(content, parsed.content);
}

TEST(NotamGrammar, NotamBrackets) {
    NotamGrammar grammar;
    AstNode node;

    std::string content = "AERODROME [US DOD E-IPL ONLY] VOR RWY 25L WITH JULIAN DATE OF\n\
 20358 NOT AUTHORIZED";
    std::string notam("W0095/21 NOTAMN\n\
Q) EDWW/QPIGV/IV/NBO/A/000/999/5221N01330E005 A) EDDB B) 2101281521 C) 2102250001\n\
E) " + content);

    ASSERT_TRUE(Parser::parse(notam, grammar, node));

    auto& parsed = boost::get<AstNotam>(node);
    EXPECT_EQ("W0095/21 NOTAMN", parsed.title);
    EXPECT_EQ("EDWW", parsed.info.fir);
    EXPECT_EQ("QPIGV", parsed.info.code);
    EXPECT_EQ("IV", parsed.info.flightRule);
    EXPECT_EQ("NBO", parsed.info.purpose);
    EXPECT_EQ("A", parsed.info.scope);
    EXPECT_EQ("000", parsed.info.lowerAltitude);
    EXPECT_EQ("999", parsed.info.upperAltitude);
    EXPECT_EQ("5221N01330E", parsed.info.coordinate);
    EXPECT_EQ("005", parsed.info.radius);
    EXPECT_EQ("EDDB", parsed.icao);
    EXPECT_EQ("2101281521", parsed.startTime);
    EXPECT_EQ("2102250001", parsed.endTime);
    EXPECT_EQ(0UL, parsed.dayTime.startTime.length());
    EXPECT_EQ(0UL, parsed.dayTime.endTime.length());
    EXPECT_EQ(content, parsed.content);
}
