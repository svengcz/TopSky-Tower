# Author:
#   Sven Czarnian <devel@svcz.de>
# Copyright:
#   2020-2021 Sven Czarnian
# License:
#   GNU General Public License (GPLv3)
# Brief:
#   Creates the surveillance library

SET(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/surveillance/ARIWSControl.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/CMACControl.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/DepartureModel.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/FlightPlanControl.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/MTCDControl.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/RadioControl.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/STCDControl.h
)
SET(SOURCE_FILES
    ARIWSControl.cpp
    CMACControl.cpp
    DepartureModel.cpp
    FlightPlanControl.cpp
    MTCDControl.cpp
    RadioControl.cpp
    STCDControl.cpp
)

# define the helper library
ADD_LIBRARY(
    surveillance STATIC
        ${SOURCE_FILES}
        ${HEADER_FILES}
)
TARGET_LINK_LIBRARIES(surveillance types formats system)
ADD_DEPENDENCIES(surveillance Curl GeographicLib Nanoflann Boost)

IF (CODE_ANALYSIS)
    SET_PROPERTY(TARGET surveillance PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATHS})
ENDIF ()

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
