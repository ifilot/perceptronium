/*******************************************************************************
 *
 *  Copyright (C) 2024 Ivo Filot
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

/**
 * @file
 * @brief CppUnit fixture for parser-related unit tests.
 */

#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "../datafile.h"
#include "../requestfile.h"
#include "../datafile_parser.h"

class TestParser : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestParser);
    CPPUNIT_TEST(testParseDataFile);
    CPPUNIT_TEST(testParsePackageFile);
    CPPUNIT_TEST_SUITE_END();

public:
    /** @brief Prepare test fixtures before each test case. */
    void setUp();

    /** @brief Clean up test fixtures after each test case. */
    void tearDown();

    /** @brief Validate parsing of a single `.data` structure file. */
    void testParseDataFile();

    /** @brief Validate parsing of multiple entries from a binary package file. */
    void testParsePackageFile();
};
