#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/permut/swap_perturbation.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Permutation operators");

TEST_CASE("Swap perturbation") {
  SUBCASE("success #1") {
    TestRNG rng({}, {}, {{1, 3}});
    metah::permut::SwapPerturbation swap(rng);

    std::vector<size_t> genotype{0, 1, 2, 3, 4};
    std::vector<size_t> result;
    swap.perturb(genotype, result);
    CHECK_EQ(result, std::vector<size_t>{0, 3, 2, 1, 4});
  }

  SUBCASE("success #2") {
    TestRNG rng({}, {}, {{0, 4}});
    metah::permut::SwapPerturbation swap(rng);

    std::vector<size_t> genotype{0, 1, 2, 3, 4};
    std::vector<size_t> result;
    swap.perturb(genotype, result);
    CHECK_EQ(result, std::vector<size_t>{4, 1, 2, 3, 0});
  }
}

TEST_SUITE_END();
