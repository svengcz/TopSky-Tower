# Author:
#   Sven Czarnian <devel@svcz.de>
# Copyright:
#   2020-2021 Sven Czarnian
# License:
#   GNU General Public License (GPLv3)
# Brief:
#   Creates the management library

SET(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/management/DepartureSequenceControl.h
    ${CMAKE_SOURCE_DIR}/include/management/HoldingPointMap.h
    ${CMAKE_SOURCE_DIR}/include/management/Notam.h
    ${CMAKE_SOURCE_DIR}/include/management/NotamControl.h
    ${CMAKE_SOURCE_DIR}/include/management/PdcControl.h
    ${CMAKE_SOURCE_DIR}/include/management/SectorControl.h
    ${CMAKE_SOURCE_DIR}/include/management/StandControl.h
)
SET(SOURCE_GRAMMAR_FILES
    grammar/AbstractSyntaxTree.hpp
    grammar/AstBase.hpp
    grammar/AstNotam.hpp
    grammar/AstRunway.hpp
    grammar/AstStand.hpp
    grammar/AstTransformation.hpp
    grammar/Notam.hpp
    grammar/Parser.hpp
    grammar/Runway.hpp
    grammar/Stand.hpp
)
SET(SOURCE_FILES
    DepartureSequenceControl.cpp
    PdcControl.cpp
    NotamControl.cpp
    SectorControl.cpp
    StandControl.cpp
)

# define the helper library
ADD_LIBRARY(
    management STATIC
        ${SOURCE_FILES}
        ${SOURCE_GRAMMAR_FILES}
        ${HEADER_FILES}
)
TARGET_LINK_LIBRARIES(management types formats system)
ADD_DEPENDENCIES(management Curl GeographicLib Nanoflann)

IF (CODE_ANALYSIS)
    SET_PROPERTY(TARGET helper PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATHS})
ENDIF ()

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Source Files\\grammar" FILES ${SOURCE_GRAMMAR_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
