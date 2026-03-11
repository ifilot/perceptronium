#include "test_requestfile.h"

CPPUNIT_TEST_SUITE_REGISTRATION(TestRequestFile);

void TestRequestFile::testCountsSortingAndNormalization() {
    RequestFile request("request_unsorted_valid");

    CPPUNIT_ASSERT_EQUAL(3, request.count_G0());
    CPPUNIT_ASSERT_EQUAL(3, request.count_G1());
    CPPUNIT_ASSERT_EQUAL(2, request.count_G2());
    CPPUNIT_ASSERT_EQUAL(1, request.count_G3());
    CPPUNIT_ASSERT_EQUAL(1, request.count_G3f());
    CPPUNIT_ASSERT_EQUAL(1, request.count_G4());
    CPPUNIT_ASSERT_EQUAL(1, request.count_G5());

    // G0 is signed and clamped to +/-1 via halfMaxAbsHeavyside * CFLOATY_min(abs(value)).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, request.get_G0(0), 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, request.get_G0(1), 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25, request.get_G0(2), 1e-12);

    // G1 is sorted in ascending order after parse.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.5, request.get_G1(0), 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, request.get_G1(1), 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, request.get_G1(2), 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, request.get_max_cutoff(), 1e-12);

    auto g2_0 = request.get_G2(0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, g2_0.eta, 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.2, g2_0.Rs, 1e-12);

    auto g4_0 = request.get_G4(0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.7, g4_0.eta, 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, g4_0.zeta, 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, g4_0.lambda, 1e-12);

    auto g5_0 = request.get_G5(0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.3, g5_0.eta, 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, g5_0.zeta, 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, g5_0.lambda, 1e-12);

    const int expected_symmetry_functions = request.count_G0() +
        request.count_G1() * (1 + request.count_G2() + request.count_G3() +
                              request.count_G3f() + request.count_G4() + request.count_G5());
    CPPUNIT_ASSERT_EQUAL(expected_symmetry_functions, request.get_nr_symmetry_functions());
}

void TestRequestFile::testOutOfRangeAccessThrows() {
    RequestFile request("request_unsorted_valid");

    CPPUNIT_ASSERT_THROW(request.get_G0(request.count_G0()), std::out_of_range);
    CPPUNIT_ASSERT_THROW(request.get_G1(request.count_G1()), std::out_of_range);
    CPPUNIT_ASSERT_THROW(request.get_G2(request.count_G2()), std::out_of_range);
    CPPUNIT_ASSERT_THROW(request.get_G3(request.count_G3()), std::out_of_range);
    CPPUNIT_ASSERT_THROW(request.get_G3f(request.count_G3f()), std::out_of_range);
    CPPUNIT_ASSERT_THROW(request.get_G4(request.count_G4()), std::out_of_range);
    CPPUNIT_ASSERT_THROW(request.get_G5(request.count_G5()), std::out_of_range);
}

void TestRequestFile::testInvalidRowsAreIgnored() {
    RequestFile request("request_invalid_rows");

    // Invalid lines are caught internally and should not contribute to counts.
    CPPUNIT_ASSERT_EQUAL(1, request.count_G0());
    CPPUNIT_ASSERT_EQUAL(2, request.count_G1());
    CPPUNIT_ASSERT_EQUAL(1, request.count_G2());
    CPPUNIT_ASSERT_EQUAL(0, request.count_G3());
    CPPUNIT_ASSERT_EQUAL(0, request.count_G3f());
    CPPUNIT_ASSERT_EQUAL(1, request.count_G4());
    CPPUNIT_ASSERT_EQUAL(1, request.count_G5());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, request.get_G1(0), 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, request.get_G1(1), 1e-12);
}

void TestRequestFile::testMissingFileThrows() {
    CPPUNIT_ASSERT_THROW(RequestFile("definitely_missing_request_file"), std::runtime_error);
}
