#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/permut/ox.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Permutation operators");

TEST_CASE("OX crossover") {
  std::vector<size_t> parent1{2, 3, 7, 1, 6, 0, 5, 4};
  std::vector<size_t> parent2{3, 1, 4, 0, 5, 7, 2, 6};
  std::vector<size_t> child1(8);
  std::vector<size_t> child2(8);

  SUBCASE("success") {
    TestRNG rng({}, {0.0f}, {{3, 5}});
    metah::permut::Ox ox(0.5f, rng);

    CHECK(ox.cross(parent1, parent2, child1, child2) == true);
    CHECK_EQ(child1, std::vector<size_t>{4, 5, 7, 1, 6, 0, 2, 3});
    CHECK_EQ(child2, std::vector<size_t>{3, 1, 6, 0, 5, 7, 4, 2});
  }

  SUBCASE("failure") {
    TestRNG rng({}, {0.6f}, {});
    metah::permut::Ox ox(0.5f, rng);

    CHECK(ox.cross(parent1, parent2, child1, child2) == false);
    CHECK_EQ(child1, parent1);
    CHECK_EQ(child2, parent2);
  }
}

TEST_SUITE_END();
