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

#include <system/FlightRegistry.h>

using namespace topskytower;

TEST(FlightRegistry, FlightExists) {
    EXPECT_FALSE(system::FlightRegistry::instance().flightExists("TEST"));
}
