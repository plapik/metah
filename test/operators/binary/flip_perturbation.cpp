#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/binary/flip_perturbation.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Binary operators");

TEST_CASE("Single flip perturbation") {
  SUBCASE("success #1") {
    TestRNG rng({1}, {}, {});
    metah::binary::SingleFlipPerturbation<std::vector<uint8_t>, TestRNG> flip(
        rng);

    std::vector<uint8_t> genotype{1, 0, 1, 0, 1};
    std::vector<uint8_t> result;
    flip.perturb(genotype, result);
    CHECK_EQ(result, std::vector<uint8_t>{1, 1, 1, 0, 1});
  }

  SUBCASE("success #2") {
    TestRNG rng({3}, {}, {});
    metah::binary::SingleFlipPerturbation<std::vector<uint8_t>, TestRNG> flip(
        rng);

    std::vector<uint8_t> genotype{1, 0, 1, 0, 1};
    std::vector<uint8_t> result;
    flip.perturb(genotype, result);
    CHECK_EQ(result, std::vector<uint8_t>{1, 0, 1, 1, 1});
  }
}

TEST_CASE("Flip perturbation") {
  SUBCASE("success") {
    TestRNG rng({}, {}, {}, {1, 0, 1});
    metah::binary::FlipPerturbation<std::vector<uint8_t>, TestRNG> flip(0.5,
                                                                        rng);

    std::vector<uint8_t> genotype{0, 0, 0, 0, 0};
    std::vector<uint8_t> result;
    flip.perturb(genotype, result);
    CHECK_EQ(result, std::vector<uint8_t>{0, 1, 1, 0, 1});
  }
}

TEST_CASE("Flip perturbation (std::vector<bool>)") {
  SUBCASE("success") {
    TestRNG rng({}, {}, {}, {1, 0, 1});
    metah::binary::FlipPerturbation<std::vector<bool>, TestRNG> flip(0.5, rng);

    std::vector<bool> genotype(5, false);
    std::vector<bool> result(5, false);
    flip.perturb(genotype, result);

    CHECK(result[0] == false);
    CHECK(result[1] == true);
    CHECK(result[2] == true);
    CHECK(result[3] == false);
    CHECK(result[4] == true);
  }
}

TEST_SUITE_END();
