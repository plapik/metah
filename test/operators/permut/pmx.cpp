#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/permut/pmx.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Permutation operators");

TEST_CASE("PMX crossover") {
  SUBCASE("success") {
    TestRNG rng({}, {0.0f}, {{2, 5}});
    metah::permut::Pmx pmx(0.5f, rng);

    std::vector<size_t> parent1{0, 1, 2, 3, 4, 5, 6};
    std::vector<size_t> parent2{4, 3, 5, 6, 1, 0, 2};
    std::vector<size_t> child1(7);
    std::vector<size_t> child2(7);
    CHECK(pmx.cross(parent1, parent2, child1, child2) == true);
    CHECK_EQ(child1, std::vector<size_t>{2, 4, 5, 6, 1, 0, 3});
    CHECK_EQ(child2, std::vector<size_t>{1, 6, 2, 3, 4, 5, 0});
  }

  SUBCASE("failure") {
    TestRNG rng({}, {0.6f}, {});
    metah::permut::Pmx pmx(0.5f, rng);

    std::vector<size_t> parent1{2, 3, 7, 1, 6, 0, 5, 4};
    std::vector<size_t> parent2{3, 1, 4, 0, 5, 7, 2, 6};
    std::vector<size_t> child1(8);
    std::vector<size_t> child2(8);
    CHECK(pmx.cross(parent1, parent2, child1, child2) == false);
    CHECK_EQ(child1, parent1);
    CHECK_EQ(child2, parent2);
  }
}

TEST_SUITE_END();