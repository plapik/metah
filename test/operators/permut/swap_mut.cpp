#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/permut/swap_mut.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Permutation operators");

TEST_CASE("Swap mutation") {
  const std::vector<size_t> base{0, 1, 2, 3, 4};

  SUBCASE("success #1") {
    TestRNG rng({}, {0.0f}, {{1, 0}});
    metah::permut::Swap swap(0.5f, rng);

    auto genotype = base;
    CHECK(swap.mutate(genotype) == true);
    CHECK_EQ(genotype, std::vector<size_t>{1, 0, 2, 3, 4});
  }

  SUBCASE("success #2") {
    TestRNG rng({}, {0.0f}, {{0, 1}});
    metah::permut::Swap swap(0.5f, rng);

    auto genotype = base;
    CHECK(swap.mutate(genotype) == true);
    CHECK_EQ(genotype, std::vector<size_t>{1, 0, 2, 3, 4});
  }

  SUBCASE("failure") {
    TestRNG rng({}, {0.6f}, {});
    metah::permut::Swap swap(0.5f, rng);

    auto genotype = base;
    CHECK(swap.mutate(genotype) == false);
    CHECK_EQ(genotype, base);
  }
}

TEST_SUITE_END();