#pragma once

#include <cppunit/extensions/HelperMacros.h>

#include "../datafile_parser.h"

class TestDataFileParser : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestDataFileParser);
    CPPUNIT_TEST(testMissingFileReturnsEmptyResult);
    CPPUNIT_TEST(testParsesAllPackageEntries);
    CPPUNIT_TEST_SUITE_END();

public:
    void testMissingFileReturnsEmptyResult();
    void testParsesAllPackageEntries();
};
