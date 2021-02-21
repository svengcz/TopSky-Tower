# Author:
#   Sven Czarnian <devel@svcz.de>
# License:
#   Closed Source
# Brief:
#   Creates a GatePlanner project for EuroScope

CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

# define the project
PROJECT(TopSky-Tower LANGUAGES CXX VERSION "0.8.1")
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
IF (MSVC)
    IF (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    ELSE ()
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    ENDIF ()
    IF (NOT CMAKE_CXX_FLAGS MATCHES "/MP")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP")
    ENDIF ()

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /sdl /permissive-")
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /sdl /permissive-")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFESTUAC:NO")
    ADD_DEFINITIONS(/D_USRDLL)
ENDIF ()

# define the options
OPTION(BUILD_TESTS "Build the tests and test framework" OFF)

# add the binary, include and installation directories to the search paths
INCLUDE_DIRECTORIES(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_INSTALL_PREFIX}/include
    ${CMAKE_INSTALL_PREFIX}/include/eigen3
    ${PROJECT_BINARY_DIR}/boost
)
LINK_DIRECTORIES(
    ${CMAKE_INSTALL_PREFIX}/lib
)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

INCLUDE(ExternalProject)
# add third-party-dependencies
ExternalProject_Add(
    Eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG 3.3.7
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DEIGEN_TEST_CXX11=OFF
        -DEIGEN_BUILD_BTL=OFF
        -DBUILD_TESTING=OFF
)
ExternalProject_Add(
    GeographicLib
    GIT_REPOSITORY https://git.code.sf.net/p/geographiclib/code
    GIT_TAG release
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCPACK_BINARY_NSIS=OFF
        -DGEOGRAPHICLIB_DATA=${CMAKE_INSTALL_PREFIX}
        -DGEOGRAPHICLIB_TYPE=STATIC
        -DGEOGRAPHICLIB_PRECISION=1
)
ExternalProject_Add(
    Nanoflann
    GIT_REPOSITORY https://github.com/jlblancoc/nanoflann.git
    GIT_TAG v1.3.2
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_EXAMPLES=OFF
        -DBUILD_BENCHMARKS=OFF
)
ExternalProject_Add(
    Boost
    URL https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.bz2
    URL_HASH SHA256=953db31e016db7bb207f11432bef7df100516eeb746843fa0486a222e3fd49cb
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    SOURCE_DIR ${PROJECT_BINARY_DIR}/boost
)
ExternalProject_Add(
    Curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-7_71_1
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DPICKY_COMPILER=OFF
        -DBUILD_CURL_EXE=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DENABLE_ARES=OFF
        -DENABLE_DEBUG=OFF
        -DENABLE_CURLDEBUG=OFF
        -DHTTP_ONLY=ON
        -DCURL_DISABLE_COOKIES=ON
        -DCURL_DISABLE_CRYPTO_AUTH=ON
        -DCURL_DISABLE_VERBOSE_STRINGS=ON
        -DCMAKE_USE_WINSSL=ON
)

# place the external libraries in own folders
SET_TARGET_PROPERTIES(Eigen         PROPERTIES FOLDER "external")
SET_TARGET_PROPERTIES(GeographicLib PROPERTIES FOLDER "external")
SET_TARGET_PROPERTIES(Nanoflann     PROPERTIES FOLDER "external")
SET_TARGET_PROPERTIES(Boost         PROPERTIES FOLDER "external")
SET_TARGET_PROPERTIES(Curl          PROPERTIES FOLDER "external")

# integrate the components
ADD_SUBDIRECTORY(helper)
ADD_SUBDIRECTORY(types)
ADD_SUBDIRECTORY(formats)
ADD_SUBDIRECTORY(system)
ADD_SUBDIRECTORY(management)
ADD_SUBDIRECTORY(surveillance)
IF (NOT ${BUILD_TESTS})
    ADD_SUBDIRECTORY(euroscope)
ELSE ()
    ADD_SUBDIRECTORY(tests)
ENDIF ()

# generate the documentation target
FIND_PACKAGE(Doxygen REQUIRED dot)

CONFIGURE_FILE(
    ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
)

ADD_CUSTOM_TARGET(
    doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API-documentation with Doxygen"
    VERBATIM
)
