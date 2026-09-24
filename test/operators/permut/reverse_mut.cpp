#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/permut/reverse_mut.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Permutation operators");

TEST_CASE("Reverse mutation") {
  SUBCASE("success #1") {
    TestRNG rng({}, {0.0f}, {{3, 0}});
    metah::permut::Reverse reverse(0.5f, rng);

    std::vector<size_t> genotype{0, 1, 2, 3, 4};
    CHECK(reverse.mutate(genotype) == true);
    CHECK_EQ(genotype, std::vector<size_t>{3, 2, 1, 0, 4});
  }

  SUBCASE("success #2") {
    TestRNG rng({}, {0.0f}, {{0, 3}});
    metah::permut::Reverse reverse(0.5f, rng);

    std::vector<size_t> genotype{0, 1, 2, 3, 4};
    CHECK(reverse.mutate(genotype) == true);
    CHECK_EQ(genotype, std::vector<size_t>{3, 2, 1, 0, 4});
  }

  SUBCASE("failure") {
    TestRNG rng({}, {0.6f}, {});
    metah::permut::Reverse reverse(0.5f, rng);

    std::vector<size_t> genotype{0, 1, 2, 3, 4};
    CHECK(reverse.mutate(genotype) == false);
    CHECK_EQ(genotype, std::vector<size_t>{0, 1, 2, 3, 4});
  }
}

TEST_SUITE_END();