# Author:
#   Sven Czarnian <devel@svcz.de>
# License:
#   Closed Source
# Brief:
#   Creates the management library

SET(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/management/StandControl.h
    ${CMAKE_SOURCE_DIR}/include/management/PdcControl.h
)
SET(SOURCE_FILES
    StandControl.cpp
    PdcControl.cpp
)

# define the helper library
ADD_LIBRARY(
    management STATIC
        ${SOURCE_FILES}
        ${HEADER_FILES}
)
TARGET_LINK_LIBRARIES(management types formats system)
ADD_DEPENDENCIES(management Curl GeographicLib Nanoflann)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
