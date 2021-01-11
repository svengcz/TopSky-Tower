# Author:
#   Sven Czarnian <devel@svcz.de>
# License:
#   Closed Source
# Brief:
#   Creates the surveillance library

SET(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/surveillance/ARIWSControl.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/FlightPlanControl.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/MTCAControl.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/PdcControl.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/SectorControl.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/StandControl.h
)
SET(SOURCE_FILES
    ARIWSControl.cpp
    FlightPlanControl.cpp
    MTCAControl.cpp
    PdcControl.cpp
    SectorControl.cpp
    StandControl.cpp
)

# define the helper library
ADD_LIBRARY(
    surveillance STATIC
        ${SOURCE_FILES}
        ${HEADER_FILES}
)
TARGET_LINK_LIBRARIES(surveillance types formats system)
ADD_DEPENDENCIES(surveillance Curl GeographicLib Nanoflann)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
