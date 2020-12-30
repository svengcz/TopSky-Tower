# Author:
#   Sven Czarnian <devel@svcz.de>
# License:
#   Closed Source
# Brief:
#   Creates the types library

SET(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/types/AirportConfiguration.h
    ${CMAKE_SOURCE_DIR}/include/types/ControllerInfo.h
    ${CMAKE_SOURCE_DIR}/include/types/Coordinate.h
    ${CMAKE_SOURCE_DIR}/include/types/Flight.h
    ${CMAKE_SOURCE_DIR}/include/types/Position.h
    ${CMAKE_SOURCE_DIR}/include/types/Quantity.hpp
    ${CMAKE_SOURCE_DIR}/include/types/Sector.h
    ${CMAKE_SOURCE_DIR}/include/types/SectorBorder.h
    ${CMAKE_SOURCE_DIR}/include/types/SystemConfiguration.h
)
SET(SOURCE_FILES
    ControllerInfo.cpp
    Coordinate.cpp
    Flight.cpp
    Position.cpp
    Sector.cpp
    SectorBorder.cpp
)

# define the helper library
ADD_LIBRARY(
    types STATIC
        ${SOURCE_FILES}
        ${HEADER_FILES}
)
TARGET_LINK_LIBRARIES(types helper)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
