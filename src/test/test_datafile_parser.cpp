#include "test_datafile_parser.h"

CPPUNIT_TEST_SUITE_REGISTRATION(TestDataFileParser);

void TestDataFileParser::testMissingFileReturnsEmptyResult() {
    const auto result = DataFileParser::read_multiple_from_binary("missing.pkg");
    CPPUNIT_ASSERT(result.empty());
}

void TestDataFileParser::testParsesAllPackageEntries() {
    const auto result = DataFileParser::read_multiple_from_binary("ch4.pkg");

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(-17.874661410, result[0].Energy, 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-20.299238930, result[1].Energy, 1e-9);

    CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(3), result[0].Periodicity);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(3), result[1].Periodicity);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), result[0].Atomlist.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), result[1].Atomlist.size());

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), result[0].Coordinates.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), result[1].Coordinates.size());

    // Spot-check one coordinate in each entry.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result[0].Coordinates[0][0], 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result[0].Coordinates[0][1], 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result[0].Coordinates[0][2], 1e-9);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.2536664, result[1].Coordinates[2][0], 1e-5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.6557512, result[1].Coordinates[2][1], 1e-5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.1474598, result[1].Coordinates[2][2], 1e-5);

    // Lattice should remain 10x10x10 cube for both entries.
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            const double expected = (i == j) ? 10.0 : 0.0;
            CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, result[0].Lattice[i][j], 1e-9);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, result[1].Lattice[i][j], 1e-9);
        }
    }
}
