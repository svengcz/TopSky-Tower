# Author:
#   Sven Czarnian <devel@svcz.de>
# License:
#   Closed Source
# Brief:
#   Creates the formats library

SET(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/formats/AirportFileFormat.h
    ${CMAKE_SOURCE_DIR}/include/formats/EseFileFormat.h
    ${CMAKE_SOURCE_DIR}/include/formats/SettingsFileFormat.h
)
SET(SOURCE_FILES
    AirportFileFormat.cpp
    EseFileFormat.cpp
    IniFileFormat.cpp
    IniFileFormat.h
    SettingsFileFormat.cpp
)

# define the helper library
ADD_LIBRARY(
    formats STATIC
        ${SOURCE_FILES}
        ${HEADER_FILES}
)
TARGET_LINK_LIBRARIES(formats types)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
