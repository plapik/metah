#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/tournament.hpp>
#include <vector>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Selection operators");

TEST_CASE("Tournament selection") {
  const std::vector<int> scores{5, 2, 8, 1, 9};

  SUBCASE("minimizing") {
    TestRNG rng({0, 3, 2}, {}, {});
    metah::Tournament<int, true, TestRNG> tournament(3, rng);

    CHECK_EQ(tournament.select(scores), 3);
  }

  SUBCASE("maximizing") {
    TestRNG rng({0, 3, 2}, {}, {});
    metah::Tournament<int, false, TestRNG> tournament(3, rng);

    CHECK_EQ(tournament.select(scores), 2);
  }

  SUBCASE("single competitor") {
    TestRNG rng({4}, {}, {});
    metah::Tournament<int, true, TestRNG> tournament(1, rng);

    CHECK_EQ(tournament.select(scores), 4);
  }
}

TEST_SUITE_END();
