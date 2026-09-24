#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/permut/swap_nbr.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Permutation operators");

TEST_CASE("Swap neighbour") {
  SUBCASE("success #1") {
    TestRNG rng({}, {}, {{1, 3}});
    metah::permut::RngSwapNeighbour swap(10, rng);

    std::vector<size_t> original{0, 1, 2, 3, 4};
    std::vector<size_t> target(5);
    const auto move = swap.generate(original, target, 0);
    CHECK_EQ(target, std::vector<size_t>{0, 3, 2, 1, 4});
    CHECK_EQ(move.i, 1);
    CHECK_EQ(move.j, 3);
  }

  SUBCASE("success #2") {
    TestRNG rng({}, {}, {{0, 4}});
    metah::permut::RngSwapNeighbour swap(10, rng);

    std::vector<size_t> original{0, 1, 2, 3, 4};
    std::vector<size_t> target(5);
    swap.generate(original, target, 0);
    CHECK_EQ(target, std::vector<size_t>{4, 1, 2, 3, 0});
  }

  SUBCASE("size") {
    TestRNG rng({}, {}, {{0, 1}});
    metah::permut::RngSwapNeighbour swap(10, rng);

    std::vector<size_t> original{0, 1, 2, 3, 4};
    CHECK_EQ(swap.size(original), 10);
  }
}

TEST_SUITE_END();
