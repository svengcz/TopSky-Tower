# Author:
#   Sven Czarnian <devel@svcz.de>
# License:
#   Closed Source
# Brief:
#   Creates the surveillance library

SET(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/surveillance/Controller.h
)
SET(SOURCE_FILES
    Controller.cpp
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
