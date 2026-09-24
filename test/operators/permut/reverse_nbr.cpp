#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/permut/reverse_nbr.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Permutation operators");

TEST_CASE("Reverse neighbour") {
  SUBCASE("success #1") {
    TestRNG rng({}, {}, {{3, 0}});
    metah::permut::ReverseNeighbour reverse(40, rng);

    std::vector<size_t> genotype{0, 1, 2, 3, 4};
    std::vector<size_t> target(5);
    reverse.generate(genotype, target, 0);
    CHECK_EQ(target, std::vector<size_t>{3, 2, 1, 0, 4});
  }
}

TEST_SUITE_END();