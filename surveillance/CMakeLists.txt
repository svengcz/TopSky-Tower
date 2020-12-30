# Author:
#   Sven Czarnian <devel@svcz.de>
# License:
#   Closed Source
# Brief:
#   Creates the surveillance library

SET(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/surveillance/ConfigurationRegistry.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/FlightRegistry.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/PdcControl.h
    ${CMAKE_SOURCE_DIR}/include/surveillance/SectorControl.h
)
SET(SOURCE_FILES
    ConfigurationRegistry.cpp
    FlightRegistry.cpp
    PdcControl.cpp
    SectorControl.cpp
)

# define the helper library
ADD_LIBRARY(
    surveillance STATIC
        ${SOURCE_FILES}
        ${HEADER_FILES}
)
TARGET_LINK_LIBRARIES(surveillance types formats)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
