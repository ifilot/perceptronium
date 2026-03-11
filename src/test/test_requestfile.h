#pragma once

#include <cppunit/extensions/HelperMacros.h>

#include "../requestfile.h"

class TestRequestFile : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestRequestFile);
    CPPUNIT_TEST(testCountsSortingAndNormalization);
    CPPUNIT_TEST(testOutOfRangeAccessThrows);
    CPPUNIT_TEST(testInvalidRowsAreIgnored);
    CPPUNIT_TEST(testMissingFileThrows);
    CPPUNIT_TEST_SUITE_END();

public:
    void testCountsSortingAndNormalization();
    void testOutOfRangeAccessThrows();
    void testInvalidRowsAreIgnored();
    void testMissingFileThrows();
};
