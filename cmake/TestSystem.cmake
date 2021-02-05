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

# download googletest
ExternalProject_Add(
    GTest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.10.0
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -Dgtest_force_shared_crt=ON
        -DBUILD_SHARED_LIBS=OFF
)
SET_TARGET_PROPERTIES(GTest PROPERTIES FOLDER "external")

# create the command to run the tests
IF (CMAKE_CONFIGURATION_TYPES)
    ADD_CUSTOM_TARGET(tests COMMAND ${CMAKE_CTEST_COMMAND} 
        --force-new-ctest-process --output-on-failure 
        --build-config "$<CONFIGURATION>")
ELSE()
    ADD_CUSTOM_TARGET(tests COMMAND ${CMAKE_CTEST_COMMAND} 
        --force-new-ctest-process --output-on-failure)
ENDIF()
SET_TARGET_PROPERTIES(tests PROPERTIES FOLDER "tests")

MACRO(AddTest TESTNAME FILES LIBRARIES TEST_WORKING_DIRECTORY)
    ADD_EXECUTABLE(${TESTNAME} ${FILES})
    TARGET_LINK_LIBRARIES(${TESTNAME} ${LIBRARIES})
    TARGET_LINK_LIBRARIES(${TESTNAME} debug gtest_maind.lib optimized gtest_main.lib)
    TARGET_LINK_LIBRARIES(${TESTNAME} debug gtestd.lib optimized gtest.lib)
    gtest_add_tests(TARGET ${TESTNAME})
    SET_TARGET_PROPERTIES(${TESTNAME} PROPERTIES FOLDER tests)
ENDMACRO()

MACRO(AddTest TESTNAME FILES TEST_WORKING_DIRECTORY)
    ADD_EXECUTABLE(${TESTNAME} ${FILES})
    TARGET_LINK_LIBRARIES(${TESTNAME} debug gtest_maind.lib optimized gtest_main.lib)
    TARGET_LINK_LIBRARIES(${TESTNAME} debug gtestd.lib optimized gtest.lib)
    gtest_add_tests(TARGET ${TESTNAME})
    SET_TARGET_PROPERTIES(${TESTNAME} PROPERTIES FOLDER tests)
ENDMACRO()
