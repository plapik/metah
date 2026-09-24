#include <doctest/doctest.h>

#include <metah/operators/binary/bitset_single_point_cross.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Binary operators");

TEST_CASE("BitsetSinglePointCrossover success") {
  struct test_param {
    size_t bitset_size;
    size_t split_point;
  };

  // Test parameters designed to hit specific DynamicBitset<uint64_t> edge
  // cases:
  auto [bitset_size, split_point] = GENERATE(
      test_param{20, 8},     // Sub-block split
      test_param{128, 64},   // Exact block boundary split (end of block 1)
      test_param{192, 96},   // Mid-block split (middle of block 2)
      test_param{192, 128},  // Exact block boundary (end of block 2)
      test_param{192, 10},   // Split in the first block
      test_param{192, 190});   // Split in the very last block

  // Using default uint64_t block size
  using Bitset = metah::DynamicBitset<>;

  Bitset p1_zeros(bitset_size);
  p1_zeros.reset();

  Bitset p2_ones(bitset_size);
  p2_ones.set();

  Bitset c1(bitset_size);
  Bitset c2(bitset_size);

  Bitset expected_c1(bitset_size);
  expected_c1.reset();

  Bitset expected_c2(bitset_size);
  expected_c2.reset();

  expected_c1.set(split_point, bitset_size - split_point);
  expected_c2.set(0, split_point);

  TestRNG rng({split_point}, {0.4f}, {});

  metah::binary::BitsetSinglePointCrossover<Bitset, TestRNG> spc(0.5, rng);

  CHECK(spc.cross(p1_zeros, p2_ones, c1, c2));
  CHECK(c1 == expected_c1);
  CHECK(c2 == expected_c2);
}

TEST_CASE("BitsetSinglePointCrossover failure on probability") {
  using Bitset = metah::DynamicBitset<>;
  size_t bitset_size = 64;

  Bitset p1(bitset_size);
  Bitset p2(bitset_size);
  Bitset c1(bitset_size);
  Bitset c2(bitset_size);

  TestRNG rng({32}, {0.9f}, {});
  metah::binary::BitsetSinglePointCrossover<Bitset, TestRNG> spc(0.5, rng);

  CHECK_FALSE(spc.cross(p1, p2, c1, c2));
}

TEST_SUITE_END();
