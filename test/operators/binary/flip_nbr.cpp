#include <vector>

#include <doctest/doctest.h>

#include <metah/operators/binary/flip_nbr.hpp>

#include "test_rng.hpp"

TEST_SUITE_BEGIN("Binary operators");

TEST_CASE("Single flip neighbour") {
  SUBCASE("success #1") {
    TestRNG rng({1}, {}, {});
    metah::binary::SingleFlipNeighbour<std::vector<uint8_t>, TestRNG> flip(10,
                                                                           rng);

    std::vector<uint8_t> original{1, 0, 1, 0, 1};
    std::vector<uint8_t> target(5);
    const auto move = flip.generate(original, target, 0);
    CHECK_EQ(target, std::vector<uint8_t>{1, 1, 1, 0, 1});
    CHECK_EQ(move.index, 1);
  }

  SUBCASE("success #2") {
    TestRNG rng({3}, {}, {});
    metah::binary::SingleFlipNeighbour<std::vector<uint8_t>, TestRNG> flip(10,
                                                                           rng);

    std::vector<uint8_t> original{1, 0, 1, 0, 1};
    std::vector<uint8_t> target(5);
    flip.generate(original, target, 0);
    CHECK_EQ(target, std::vector<uint8_t>{1, 0, 1, 1, 1});
  }

  SUBCASE("size") {
    TestRNG rng({0}, {}, {});
    metah::binary::SingleFlipNeighbour<std::vector<uint8_t>, TestRNG> flip(10,
                                                                           rng);

    std::vector<uint8_t> original{1, 0, 1, 0, 1};
    CHECK_EQ(flip.size(original), 10);
  }
}

TEST_CASE("Flip neighbour") {
  SUBCASE("success") {
    TestRNG rng({}, {}, {}, {1, 0, 1});
    metah::binary::FlipNeighbour<std::vector<uint8_t>, TestRNG> flip(10, 0.5,
                                                                     rng);

    std::vector<uint8_t> original{0, 0, 0, 0, 0};
    std::vector<uint8_t> target(5);
    const auto move = flip.generate(original, target, 0);
    CHECK_EQ(target, std::vector<uint8_t>{0, 1, 1, 0, 1});
    CHECK_EQ(move.indices, std::vector<size_t>{1, 2, 4});
  }
}

TEST_CASE("Flip neighbour (std::vector<bool>)") {
  SUBCASE("success") {
    TestRNG rng({}, {}, {}, {1, 0, 1});
    metah::binary::FlipNeighbour<std::vector<bool>, TestRNG> flip(10, 0.5, rng);

    std::vector<bool> original(5, false);
    std::vector<bool> target(5, false);
    const auto move = flip.generate(original, target, 0);

    CHECK(target[0] == false);
    CHECK(target[1] == true);
    CHECK(target[2] == true);
    CHECK(target[3] == false);
    CHECK(target[4] == true);
    CHECK_EQ(move.indices, std::vector<size_t>{1, 2, 4});
  }
}

TEST_SUITE_END();
