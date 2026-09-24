#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/binary/single_point_cross.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Binary operators");

TEST_CASE("SinglePointCrossover standard success") {
    size_t bitset_size = 20;
    size_t split_point = 8;

    std::vector<uint8_t> p1_zeros(bitset_size), p2_ones(bitset_size, 1),
                          c1(bitset_size), c2(bitset_size),
                          expected_c1(bitset_size), expected_c2(bitset_size);

    for (size_t i = split_point; i < expected_c1.size(); ++i) {
        expected_c1[i] = 1;
    }
    for (size_t i = 0; i < split_point; ++i) {
        expected_c2[i] = 1;
    }

    TestRNG rng({split_point}, {0.4f}, {});
    metah::binary::SinglePointCrossover spc(0.5, rng);
    CHECK(spc.cross(p1_zeros, p2_ones, c1, c2));
    CHECK_EQ(c1, expected_c1);
    CHECK_EQ(c2, expected_c2);
}

TEST_SUITE_END();
