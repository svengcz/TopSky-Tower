# Author:
#   Sven Czarnian <devel@svcz.de>
# Copyright:
#   2020-2021 Sven Czarnian
# License:
#   GNU General Public License (GPLv3)
# Brief:
#   Creates the plug-in which is used by Euroscope

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
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/res/TopSky-Tower.rc.in
    ${CMAKE_BINARY_DIR}/TopSky-Tower.rc
)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

SET(SOURCE_FILES
    Converter.cpp
    Converter.h
    HiddenWindow.cpp
    HiddenWindow.h
    PlugIn.cpp
    PlugIn.h
    RadarScreen.cpp
    RadarScreen.h
    RdfIPC.cpp
    RdfIPC.h
    stdafx.cpp
    stdafx.h
    TopSkyTowerMain.cpp
    VersionChecker.cpp
    VersionChecker.h
)
SET(SOURCE_FILES_RES
    res/resource.h
    res/targetver.h
    ${CMAKE_BINARY_DIR}/TopSky-Tower.rc
)
SET(SOURCE_FILES_UI
    ui/AriwsToolbarButton.cpp
    ui/AriwsToolbarButton.h
    ui/CmacToolbarButton.cpp
    ui/CmacToolbarButton.h
    ui/ConfigurationErrorWindow.cpp
    ui/ConfigurationErrorWindow.h
    ui/DepartureSequenceWindow.cpp
    ui/DepartureSequenceWindow.h
    ui/HoppiesLogonWindow.cpp
    ui/HoppiesLogonWindow.h
    ui/InsetWindow.cpp
    ui/InsetWindow.h
    ui/IpaToolbarButton.cpp
    ui/IpaToolbarButton.h
    ui/LvpToolbarButton.cpp
    ui/LvpToolbarButton.h
    ui/MessageViewerWindow.cpp
    ui/MessageViewerWindow.h
    ui/MtcdToolbarButton.cpp
    ui/MtcdToolbarButton.h
    ui/NotamOverviewWindow.cpp
    ui/NotamOverviewWindow.h
    ui/PdcDepartureClearanceWindow.cpp
    ui/PdcDepartureClearanceWindow.h
    ui/PdcMessageViewerWindow.cpp
    ui/PdcMessageViewerWindow.h
    ui/PdcToolbarButton.cpp
    ui/PdcToolbarButton.h
    ui/StcdToolbarButton.cpp
    ui/StcdToolbarButton.h
    ui/Toolbar.cpp
    ui/Toolbar.h
    ui/UiManager.cpp
    ui/UiManager.h
)
SET(SOURCE_FILES_UI_ELEMENTS
    ui/elements/Button.cpp
    ui/elements/Button.h
    ui/elements/Checkbox.cpp
    ui/elements/Checkbox.h
    ui/elements/EditText.cpp
    ui/elements/EditText.h
    ui/elements/TableViewer.cpp
    ui/elements/TableViewer.h
    ui/elements/Text.cpp
    ui/elements/Text.h
    ui/elements/TextViewer.cpp
    ui/elements/TextViewer.h
    ui/elements/ToolbarButton.cpp
    ui/elements/ToolbarButton.h
    ui/elements/UiElement.cpp
    ui/elements/UiElement.h
)

# define the plug in
ADD_LIBRARY(
    TopSky-Tower SHARED
        ${SOURCE_FILES_RES}
        ${SOURCE_FILES_UI}
        ${SOURCE_FILES_UI_ELEMENTS}
        ${SOURCE_FILES}
)

# define the dependencies
TARGET_INCLUDE_DIRECTORIES(TopSky-Tower INTERFACE EuroScope)
TARGET_LINK_LIBRARIES(TopSky-Tower EuroScope types formats management surveillance)
TARGET_LINK_LIBRARIES(TopSky-Tower debug libcurl-d.lib optimized libcurl.lib)
TARGET_LINK_LIBRARIES(TopSky-Tower debug Geographic_d.lib optimized Geographic.lib)
TARGET_LINK_LIBRARIES(TopSky-Tower Gdiplus Shlwapi.lib Crypt32.lib Ws2_32.lib Winmm.lib)
ADD_DEPENDENCIES(TopSky-Tower Eigen)

# configure the debugger and update the linker flags
IF(MSVC)
    SET_TARGET_PROPERTIES(TopSky-Tower
        PROPERTIES
            VS_DEBUGGER_COMMAND ${EuroScope_EXECUTABLE}
            VS_DEBUGGER_WORKING_DIRECTORY ${EuroScope_DIR}
    )
    SET_TARGET_PROPERTIES(TopSky-Tower
        PROPERTIES
            LINK_FLAGS /SUBSYSTEM:WINDOWS
    )
ENDIF()

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Source Files\\res" FILES ${SOURCE_FILES_RES})
SOURCE_GROUP("Source Files\\ui" FILES ${SOURCE_FILES_UI})
SOURCE_GROUP("Source Files\\ui\\elements" FILES ${SOURCE_FILES_UI_ELEMENTS})
