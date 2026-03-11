#*****************************************************************************
#   This file is part of PERCEPTRONIUM.                                      *
#                                                                            *
#   PERCEPTRONIUM is free software: you can redistribute it and/or modify    *
#   it under the terms of the GNU General Public License as published        *
#   by the Free Software Foundation, either version 3 of the License,        *
#   or (at your option) any later version.                                   *
#                                                                            *
#   PERCEPTRONIUM is distributed in the hope that it will be useful,         *
#   but WITHOUT ANY WARRANTY; without even the implied warranty              *
#   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
#   See the GNU General Public License for more details.                     *
#                                                                            *
#   You should have received a copy of the GNU General Public License        *
#   along with this program. If not, see <https://www.gnu.org/licenses/>.    *
#                                                                            *
#*****************************************************************************

cmake_minimum_required(VERSION 3.16)

project(bpsfp VERSION 1.0.0 LANGUAGES CXX)

set(PROGNAME "BPSFP")
message(STATUS "Writing configuration file in: ${CMAKE_CURRENT_SOURCE_DIR}/config.h")
configure_file(config.h.in config.h @ONLY)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)
endif()

find_package(PkgConfig REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenMP)
pkg_check_modules(TCLAP QUIET tclap)
if(NOT TCLAP_FOUND)
    find_path(TCLAP_INCLUDE_DIRS tclap/CmdLine.h)
    if(NOT TCLAP_INCLUDE_DIRS)
        message(FATAL_ERROR "TCLAP headers not found. Install the tclap development package or provide TCLAP_INCLUDE_DIRS.")
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

add_library(bpsfp_sources STATIC)
file(GLOB BPSFP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER BPSFP_SOURCES EXCLUDE REGEX "/main\\.cpp$")
target_sources(bpsfp_sources PRIVATE ${BPSFP_SOURCES})

target_compile_features(bpsfp_sources PUBLIC cxx_std_20)
target_include_directories(bpsfp_sources
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${TCLAP_INCLUDE_DIRS}
)
target_link_libraries(bpsfp_sources
    PUBLIC
        ZLIB::ZLIB
)

if(OpenMP_CXX_FOUND)
    option(HAS_OPENMP "OpenMP enabled" ON)
    target_link_libraries(bpsfp_sources PUBLIC OpenMP::OpenMP_CXX)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bpsfp_sources PRIVATE -march=native)
endif()

add_executable(bpsfp main.cpp)
target_link_libraries(bpsfp PRIVATE bpsfp_sources)

include(CTest)
if(BUILD_TESTING AND NOT DISABLE_TEST)
    find_package(CPPUNIT REQUIRED)
    message(STATUS "[USER] Performing unit tests")
    add_subdirectory(test)
elseif(DISABLE_TEST)
    message(STATUS "[USER] Testing routine disabled")
endif()
