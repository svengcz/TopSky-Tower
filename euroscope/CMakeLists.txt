# Author:
#   Sven Czarnian <devel@svcz.de>
# License:
#   Closed Source
# Brief:
#   Creates the plug-in which is used by Euroscope

# get the build number
EXECUTE_PROCESS(
    COMMAND
        git rev-parse --short HEAD
    WORKING_DIRECTORY
        "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE
        PROJECT_VERSION_BUILD
)
STRING(REGEX REPLACE "\n$" "" PROJECT_VERSION_BUILD "${PROJECT_VERSION_BUILD}")

# register all cmake helper to find required modules and find 3rd-party components
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake")
FIND_PACKAGE(EuroScope REQUIRED)
IF(NOT EuroScope_FOUND)
    MESSAGE(FATAL_ERROR "Unablet to build without EuroScope and the EuroScope-SDK")
ENDIF()

# create the version header and version RC file
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_BINARY_DIR}/version.h
)

# enable MFC
ADD_DEFINITIONS(-D_AFXDLL)
SET(CMAKE_MFC_FLAG 2)

SET(SOURCE_FILES
    MFCApplication.cpp
    MFCApplication.h
    PlugIn.cpp
    PlugIn.h
    RadarScreen.cpp
    RadarScreen.h
)
SET(SOURCE_FILES_RES
    stdafx.cpp
    stdafx.h
    res/resource.h
    res/TopSky-Tower.rc
)

# define the plug in
ADD_LIBRARY(
    TopSky-Tower SHARED
        ${SOURCE_FILES_RES}
        ${SOURCE_FILES}
)

# define the dependencies
TARGET_INCLUDE_DIRECTORIES(TopSky-Tower INTERFACE EuroScope)
TARGET_LINK_LIBRARIES(TopSky-Tower EuroScope Gdiplus types formats surveillance)
TARGET_LINK_LIBRARIES(TopSky-Tower debug libcurl-d.lib optimized libcurl.lib)

# configure the debugger
IF(MSVC)
    SET_TARGET_PROPERTIES(TopSky-Tower
        PROPERTIES
            VS_DEBUGGER_COMMAND ${EuroScope_EXECUTABLE}
            VS_DEBUGGER_WORKING_DIRECTORY ${EuroScope_DIR}
    )
ENDIF()

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Source Files\\res" FILES ${SOURCE_FILES_RES})
