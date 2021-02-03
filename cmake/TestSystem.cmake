# Author:
#   Sven Czarnian <devel@svcz.de>
# Copyright:
#   2020-2021 Sven Czarnian
# License:
#   GNU General Public License (GPLv3)
# Brief:
#   Creates the test system

ENABLE_TESTING()
INCLUDE(GoogleTest)
INCLUDE(FetchContent)

# download googletest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.10.0
)
FetchContent_GetProperties(googletest)
IF (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    SET(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE BOOL "")
    ADD_SUBDIRECTORY(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
    UNSET(CMAKE_SUPPRESS_DEVELOPER_WARNINGS)

    SET_TARGET_PROPERTIES(gmock      PROPERTIES FOLDER "external")
    SET_TARGET_PROPERTIES(gtest      PROPERTIES FOLDER "external")
    SET_TARGET_PROPERTIES(gtest_main PROPERTIES FOLDER "external")
ENDIF ()

# create the command to run the tests
IF (CMAKE_CONFIGURATION_TYPES)
    ADD_CUSTOM_TARGET(check COMMAND ${CMAKE_CTEST_COMMAND} 
        --force-new-ctest-process --output-on-failure 
        --build-config "$<CONFIGURATION>")
ELSE()
    ADD_CUSTOM_TARGET(check COMMAND ${CMAKE_CTEST_COMMAND} 
        --force-new-ctest-process --output-on-failure)
ENDIF()

# disable individual tests
SET(GOOGLE_TEST_INDIVIDUAL OFF)

MACRO(AddTest TESTNAME FILES LIBRARIES TEST_WORKING_DIRECTORY)
    ADD_EXECUTABLE(${TESTNAME} ${FILES})
    TARGET_LINK_LIBRARIES(${TESTNAME} gtest gmock gtest_main ${LIBRARIES})
    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY ${TEST_WORKING_DIRECTORY}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${TEST_WORKING_DIRECTORY}"
    )
    SET_TARGET_PROPERTIES(${TESTNAME} PROPERTIES FOLDER tests)
ENDMACRO()

# cleanup the cache
MARK_AS_ADVANCED(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)
