# Author:
#   Sven Czarnian <devel@svcz.de>
# Copyright:
#   2020-2021 Sven Czarnian
# License:
#   GNU General Public License (GPLv3)
# Brief:
#   Creates the helper library

SET(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/helper/Exception.h
    ${CMAKE_SOURCE_DIR}/include/helper/Math.h
    ${CMAKE_SOURCE_DIR}/include/helper/String.h
    ${CMAKE_SOURCE_DIR}/include/helper/Time.h
)
SET(SOURCE_FILES
    Exception.cpp
)

# define the plug in
ADD_LIBRARY(
    helper STATIC
        ${SOURCE_FILES}
        ${HEADER_FILES}
)

IF (CODE_ANALYSIS)
    SET_PROPERTY(TARGET helper PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATHS})
ENDIF ()

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
