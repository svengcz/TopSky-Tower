# Author:
#   Sven Czarnian <devel@svcz.de>
# License:
#   Closed Source
# Brief:
#   Creates the system library

SET(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/system/ConfigurationRegistry.h
    ${CMAKE_SOURCE_DIR}/include/system/FlightRegistry.h
)
SET(SOURCE_FILES
    ConfigurationRegistry.cpp
    FlightRegistry.cpp
)

# define the helper library
ADD_LIBRARY(
    system STATIC
        ${SOURCE_FILES}
        ${HEADER_FILES}
)
TARGET_LINK_LIBRARIES(system types formats)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
