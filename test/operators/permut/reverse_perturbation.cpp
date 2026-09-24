#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/permut/reverse_perturbation.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Permutation operators");

TEST_CASE("Reverse perturbation") {
  SUBCASE("success #1") {
    TestRNG rng({}, {}, {{3, 0}});
    metah::permut::ReversePerturbation reverse(rng);

    std::vector<size_t> genotype{0, 1, 2, 3, 4};
    std::vector<size_t> result;
    reverse.perturb(genotype, result);
    CHECK_EQ(result, std::vector<size_t>{3, 2, 1, 0, 4});
  }

  SUBCASE("success #2") {
    TestRNG rng({}, {}, {{1, 3}});
    metah::permut::ReversePerturbation reverse(rng);

    std::vector<size_t> genotype{0, 1, 2, 3, 4};
    std::vector<size_t> result;
    reverse.perturb(genotype, result);
    CHECK_EQ(result, std::vector<size_t>{0, 3, 2, 1, 4});
  }
}

TEST_SUITE_END();
