# Author:
#   Sven Czarnian <devel@svcz.de>
# Copyright:
#   2020-2021 Sven Czarnian
# License:
#   GNU General Public License (GPLv3)
# Brief:
#   Creates the types library

SET(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/types/Aircraft.h
    ${CMAKE_SOURCE_DIR}/include/types/AirportConfiguration.h
    ${CMAKE_SOURCE_DIR}/include/types/ControllerInfo.h
    ${CMAKE_SOURCE_DIR}/include/types/Coordinate.h
    ${CMAKE_SOURCE_DIR}/include/types/Flight.h
    ${CMAKE_SOURCE_DIR}/include/types/FlightPlan.h
    ${CMAKE_SOURCE_DIR}/include/types/Position.h
    ${CMAKE_SOURCE_DIR}/include/types/Quantity.hpp
    ${CMAKE_SOURCE_DIR}/include/types/Route.h
    ${CMAKE_SOURCE_DIR}/include/types/RuntimeConfiguration.h
    ${CMAKE_SOURCE_DIR}/include/types/Runway.h
    ${CMAKE_SOURCE_DIR}/include/types/Sector.h
    ${CMAKE_SOURCE_DIR}/include/types/SectorBorder.h
    ${CMAKE_SOURCE_DIR}/include/types/SystemConfiguration.h
    ${CMAKE_SOURCE_DIR}/include/types/Waypoint.h
)
SET(SOURCE_FILES
    Aircraft.cpp
    ControllerInfo.cpp
    Coordinate.cpp
    Flight.cpp
    FlightPlan.cpp
    Position.cpp
    Route.cpp
    Runway.cpp
    Sector.cpp
    SectorBorder.cpp
    Waypoint.cpp
)

# define the helper library
ADD_LIBRARY(
    types STATIC
        ${SOURCE_FILES}
        ${HEADER_FILES}
)
TARGET_LINK_LIBRARIES(types helper)
TARGET_LINK_LIBRARIES(types debug Geographic_d.lib optimized Geographic.lib)
ADD_DEPENDENCIES(types GeographicLib Boost)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
