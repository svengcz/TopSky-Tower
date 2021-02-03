# Author:
#   Sven Czarnian <devel@svcz.de>
# Copyright:
#   2020-2021 Sven Czarnian
# License:
#   GNU General Public License (GPLv3)
# Brief:
#   Creates the test system

# register all cmake helper to find required modules and find 3rd-party components
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake")
INCLUDE(TestSystem)

AddTest(FlightRegistry system/FlightRegistry.cpp system "${PROJECT_BINARY_DIR}")
