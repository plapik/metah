#include <doctest/doctest.h>

#include <metah/operators/tabu_list.hpp>

TEST_SUITE_BEGIN("Tabu lists");

TEST_CASE("FIFO evicts moves in insertion order") {
    metah::FifoTabuList<int> list(2);
    CHECK_FALSE(list.contains(1));

    list.add(1);
    list.add(2);
    CHECK(list.contains(1));
    CHECK(list.contains(2));

    list.add(3);
    CHECK_FALSE(list.contains(1));
    CHECK(list.contains(2));
    CHECK(list.contains(3));

    list.add(4);
    CHECK_FALSE(list.contains(2));
    CHECK(list.contains(3));
    CHECK(list.contains(4));
}

TEST_CASE("FIFO moves duplicates to the end at capacity") {
    metah::FifoTabuList<int> list(3);
    list.add(1);
    list.add(2);
    list.add(3);

    int refreshed = 0;
    SUBCASE("oldest move") { refreshed = 1; }
    SUBCASE("middle move") { refreshed = 2; }
    SUBCASE("newest move") { refreshed = 3; }

    list.add(refreshed);
    CHECK(list.contains(1));
    CHECK(list.contains(2));
    CHECK(list.contains(3));

    list.add(4);
    list.add(5);
    CHECK(list.contains(refreshed));
    CHECK(list.contains(4));
    CHECK(list.contains(5));

    list.add(6);
    CHECK_FALSE(list.contains(refreshed));
}

TEST_CASE("FIFO moves duplicates to the end below capacity") {
    metah::FifoTabuList<int> list(3);
    list.add(1);
    list.add(2);
    list.add(1);
    CHECK(list.contains(1));
    CHECK(list.contains(2));

    list.add(3);
    CHECK(list.contains(1));
    CHECK(list.contains(2));
    CHECK(list.contains(3));

    list.add(4);
    CHECK(list.contains(1));
    CHECK_FALSE(list.contains(2));
    CHECK(list.contains(3));
    CHECK(list.contains(4));
}

TEST_CASE("FIFO validates capacity") {
    SUBCASE("zero capacity throws") {
        CHECK_THROWS_AS(metah::FifoTabuList<int>(0), std::invalid_argument);
    }

    SUBCASE("one retains the most recent move") {
        metah::FifoTabuList<int> list(1);
        list.add(1);
        list.add(1);
        CHECK(list.contains(1));
        list.add(2);
        CHECK_FALSE(list.contains(1));
        CHECK(list.contains(2));
    }
}

TEST_CASE("FIFO accepts custom hash and equality") {
    struct Hash {
        size_t operator()(const int move) const { return move % 10; }
    };
    struct Eq {
        bool operator()(const int lhs, const int rhs) const { return lhs % 10 == rhs % 10; }
    };

    metah::FifoTabuList<int, Hash, Eq> list(1);
    metah::TabuList<int> &base = list;
    base.add(1);
    CHECK(base.contains(11));
    base.add(2);
    CHECK_FALSE(base.contains(11));
    CHECK(base.contains(12));
}

TEST_SUITE_END();
