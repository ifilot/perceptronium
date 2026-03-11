#*****************************************************************************
#   This file is part of PERCEPTRONIUM.                                      *
#                                                                             *
#   PERCEPTRONIUM is free software: you can redistribute it and/or modify     *
#   it under the terms of the GNU General Public License as published         *
#   by the Free Software Foundation, either version 3 of the License,         *
#   or (at your option) any later version.                                    *
#                                                                             *
#   PERCEPTRONIUM is distributed in the hope that it will be useful,          *
#   but WITHOUT ANY WARRANTY; without even the implied warranty               *
#   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   *
#   See the GNU General Public License for more details.                      *
#                                                                             *
#   You should have received a copy of the GNU General Public License         *
#   along with this program. If not, see <https://www.gnu.org/licenses/>.    *
#                                                                             *
#*****************************************************************************

add_library(unittest STATIC unittest.cpp)

target_include_directories(unittest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(TEST_EXECUTABLES TestParser TestRequestFile TestDataFileParser)
add_executable(TestParser test_parser.cpp)
add_executable(TestRequestFile test_requestfile.cpp)
add_executable(TestDataFileParser test_datafile_parser.cpp)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CPPUNIT_LIB ${CPPUNIT_DEBUG_LIBRARY})
else()
    set(CPPUNIT_LIB ${CPPUNIT_LIBRARY})
endif()

foreach(testexec IN LISTS TEST_EXECUTABLES)
    target_link_libraries(${testexec} PRIVATE unittest ${CPPUNIT_LIB} bpsfp_sources)
    add_test(NAME ${testexec} COMMAND ${testexec})
endforeach()

foreach(test_data request request_unsorted_valid request_invalid_rows struc_00001.data struc_00100.data ch4.pkg)
    configure_file(testinput/${test_data} ${test_data} COPYONLY)
endforeach()
