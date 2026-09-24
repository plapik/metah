#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/binary/flip_mut.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Binary operators");

TEST_CASE("Flip mutation") {
  SUBCASE("success") {
    TestRNG rng({}, {}, {}, {1, 0, 1});
    metah::binary::FlipMut<std::vector<uint8_t>, TestRNG> flip(0.5, rng);

    std::vector<uint8_t> genotype{0, 0, 0, 0, 0};
    CHECK(flip.mutate(genotype) == true);
    CHECK_EQ(genotype, std::vector<uint8_t>{0, 1, 1, 0, 1});
  }

  SUBCASE("failure") {
    TestRNG rng({}, {}, {}, {5});
    metah::binary::FlipMut<std::vector<uint8_t>, TestRNG> flip(0.5, rng);

    std::vector<uint8_t> genotype{1, 0, 1, 0, 1};
    CHECK(flip.mutate(genotype) == false);
    CHECK_EQ(genotype, std::vector<uint8_t>{1, 0, 1, 0, 1});
  }
}

TEST_CASE("Flip mutation (std::vector<bool>)") {
  SUBCASE("success") {
    TestRNG rng({}, {}, {}, {1, 0, 1});
    metah::binary::FlipMut<std::vector<bool>, TestRNG> flip(0.5, rng);

    std::vector<bool> genotype(5, false);
    CHECK(flip.mutate(genotype) == true);
    CHECK(genotype[0] == false);
    CHECK(genotype[1] == true);
    CHECK(genotype[2] == true);
    CHECK(genotype[3] == false);
    CHECK(genotype[4] == true);
  }
}

TEST_SUITE_END();
