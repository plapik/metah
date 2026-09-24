#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/permut/insert_mut.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Permutation operators");

TEST_CASE("Insert mutation") {
  std::vector<size_t> genotype{0, 1, 2, 3, 4};

  SUBCASE("success #1") {
    TestRNG rng({}, {0.0f}, {{1, 0}});
    metah::permut::Insert insert(0.5f, rng);

    CHECK(insert.mutate(genotype) == true);
    CHECK_EQ(genotype, std::vector<size_t>{1, 0, 2, 3, 4});
  }

  SUBCASE("success #2") {
    TestRNG rng({}, {0.0f}, {{0, 4}});
    metah::permut::Insert insert(0.5f, rng);

    CHECK(insert.mutate(genotype) == true);
    CHECK_EQ(genotype, std::vector<size_t>{4, 0, 1, 2, 3});
  }

  SUBCASE("failure") {
    TestRNG rng({}, {0.6f}, {});
    metah::permut::Insert insert(0.5f, rng);

    CHECK(insert.mutate(genotype) == false);
    CHECK_EQ(genotype, std::vector<size_t>{0, 1, 2, 3, 4});
  }
}

TEST_SUITE_END();