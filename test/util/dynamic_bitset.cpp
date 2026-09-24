#include <doctest/doctest.h>

#include <cstdint>
#include <metah/util/dynamic_bitset.hpp>

TEST_SUITE_BEGIN("Dynamic bitset tests");
using Bitset64 = metah::DynamicBitset<uint64_t>;
using Bitset8 = metah::DynamicBitset<uint8_t>;

TEST_CASE("Initialization and Capacity") {
  SUBCASE("Zero size") {
    Bitset64 bs(0);
    CHECK(bs.size() == 0);
    CHECK(bs.num_blocks() == 0);
    CHECK(bs.all() == true);  // An empty set trivially satisfies all()
    CHECK(bs.none() == true);
    CHECK(bs.any() == false);
  }

  SUBCASE("Non-zero size, not multiple of block size") {
    Bitset64 bs(70);
    CHECK(bs.size() == 70);
    CHECK(bs.num_blocks() == 2);  // 64 + 6 bits
    CHECK(bs.lastBlockBits() == 6);
    CHECK(bs.count() == 0);
    CHECK(bs.none() == true);
  }

  SUBCASE("Exact multiple of block size") {
    Bitset64 bs(128);
    CHECK(bs.size() == 128);
    CHECK(bs.num_blocks() == 2);
    CHECK(bs.lastBlockBits() == 0);
  }
}

TEST_CASE("Single Bit Operations and Proxy Class") {
  Bitset64 bs(100);

  SUBCASE("set(), reset(), flip(), test()") {
    bs.set(5);
    bs.set(65);  // Crosses into second block

    CHECK(bs.test(5) == true);
    CHECK(bs.test(65) == true);
    CHECK(bs.test(0) == false);
    CHECK(bs.count() == 2);

    bs.reset(5);
    CHECK(bs.test(5) == false);
    CHECK(bs.count() == 1);

    bs.flip(65);
    CHECK(bs.test(65) == false);
    CHECK(bs.none() == true);

    bs.flip(99);
    CHECK(bs.test(99) == true);
  }

  SUBCASE("operator[] proxy") {
    bs[10] = true;
    bs[75] = true;
    CHECK(bs.test(10) == true);
    CHECK(bs.test(75) == true);

    bs[10] = false;
    CHECK(bs.test(10) == false);

    bs[11] = bs[75];
    CHECK(bs.test(11) == true);

    bool val = bs[75];
    CHECK(val == true);

    bs[75].flip();
    CHECK(bs.test(75) == false);
  }
}

TEST_CASE("Range Bit Operations") {
  Bitset64 bs(200);

  SUBCASE("Same block range") {
    bs.set(10, 20);  // Bits 10 to 29
    CHECK(bs.count() == 20);
    CHECK(bs.test(9) == false);
    CHECK(bs.test(10) == true);
    CHECK(bs.test(29) == true);
    CHECK(bs.test(30) == false);

    bs.reset(15, 5);  // Bits 15 to 19
    CHECK(bs.count() == 15);
    CHECK(bs.test(14) == true);
    CHECK(bs.test(15) == false);
    CHECK(bs.test(19) == false);
    CHECK(bs.test(20) == true);

    bs.flip(10, 20);  // Flips 10-29
    CHECK(bs.count() ==
          5);  // The 5 bits we reset are now 1, the 15 we didn't are 0
  }

  SUBCASE("Cross block range") {
    bs.set(60, 10);  // Bits 60 to 69 (crosses 63|64 boundary)
    CHECK(bs.count() == 10);
    CHECK(bs.test(59) == false);
    CHECK(bs.test(60) == true);
    CHECK(bs.test(69) == true);
    CHECK(bs.test(70) == false);

    bs.reset(60, 10);
    CHECK(bs.none() == true);
  }

  SUBCASE("Multi-block range (full blocks in middle)") {
    bs.set(50, 100);  // Bits 50 to 149
    CHECK(bs.count() == 100);
    CHECK(bs.test(49) == false);
    CHECK(bs.test(50) == true);
    CHECK(bs.test(149) == true);
    CHECK(bs.test(150) == false);
  }
}

TEST_CASE("Whole Set Operations and Sanitize Check") {
  Bitset64 bs(70);  // 2 blocks, only 6 bits used in the last block

  SUBCASE("set(), all(), sanitize validation") {
    bs.set();  // Set all bits
    // If sanitize() fails, the underlying count would be 128
    CHECK(bs.count() == 70);
    CHECK(bs.all() == true);
    CHECK(bs.any() == true);
    CHECK(bs.none() == false);

    bs.reset(69);
    CHECK(bs.all() == false);  // One bit flipped off means all() is false
  }

  SUBCASE("reset(), flip()") {
    bs.set();
    bs.reset();
    CHECK(bs.none() == true);
    CHECK(bs.count() == 0);

    bs.flip();
    CHECK(bs.all() == true);
    CHECK(bs.count() == 70);
  }
}

TEST_CASE("Shift Operations") {
  Bitset64 bs(150);

  SUBCASE("Shift Left (<<)") {
    bs.set(10);
    bs <<= 5;
    CHECK(bs.test(10) == false);
    CHECK(bs.test(15) == true);

    bs <<= 60;  // Cross block boundary (15 + 60 = 75)
    CHECK(bs.test(15) == false);
    CHECK(bs.test(75) == true);
    CHECK(bs.count() == 1);  // Ensure no duplicate bits created

    bs <<= 100;  // Shift out of bounds
    CHECK(bs.none() == true);
  }

  SUBCASE("Shift Right (>>)") {
    bs.set(100);
    bs >>= 10;
    CHECK(bs.test(100) == false);
    CHECK(bs.test(90) == true);

    bs >>= 60;  // Cross block boundary (90 - 60 = 30)
    CHECK(bs.test(90) == false);
    CHECK(bs.test(30) == true);

    bs >>= 50;  // Shift out of bounds
    CHECK(bs.none() == true);
  }

  SUBCASE("Shift edge cases") {
    bs.set(63);
    bs <<= 1;
    CHECK(bs.test(64) == true);  // Exactly boundary cross

    bs.set();
    bs <<= 1;
    CHECK(bs.test(0) == false);
    CHECK(bs.count() == 149);  // Should lose exactly 1 bit
  }
}

TEST_CASE("Bitwise Binary Operators") {
  Bitset64 b1(100);
  Bitset64 b2(100);

  b1.set(10, 20);  // 10-29
  b2.set(20, 20);  // 20-39

  SUBCASE("AND (&)") {
    Bitset64 res = b1 & b2;
    CHECK(res.count() == 10);  // Intersection is 20-29
    CHECK(res.test(20) == true);
    CHECK(res.test(29) == true);
    CHECK(res.test(19) == false);
    CHECK(res.test(30) == false);
  }

  SUBCASE("OR (|)") {
    Bitset64 res = b1 | b2;
    CHECK(res.count() == 30);  // Union is 10-39
  }

  SUBCASE("XOR (^)") {
    Bitset64 res = b1 ^ b2;
    CHECK(res.count() == 20);  // 10-19 and 30-39
    CHECK(res.test(15) == true);
    CHECK(res.test(25) == false);
    CHECK(res.test(35) == true);
  }

  SUBCASE("NOT (~)") {
    Bitset64 res = ~b1;
    CHECK(res.count() == 80);  // 100 - 20
    CHECK(res.test(15) == false);
    CHECK(res.test(5) == true);
    CHECK(res.test(99) == true);
  }

  SUBCASE("Equality (==, !=)") {
    CHECK(b1 != b2);
    b2 = b1;
    CHECK(b1 == b2);

    // Ensure unused bits don't break equality
    b1.set();
    b2.set();
    CHECK(b1 == b2);
  }
}

TEST_CASE("Copy and Move Semantics (Rule of Five)") {
  Bitset64 bs1(100);
  bs1.set(50);

  SUBCASE("Copy Constructor") {
    Bitset64 bs2(bs1);
    CHECK(bs2.size() == 100);
    CHECK(bs2.test(50) == true);

    // Ensure deep copy
    bs2.reset(50);
    CHECK(bs1.test(50) == true);
  }

  SUBCASE("Copy Assignment") {
    Bitset64 bs2(50);  // Different initial size
    bs2 = bs1;
    CHECK(bs2.size() == 100);
    CHECK(bs2.test(50) == true);
  }

  SUBCASE("Move Constructor") {
    Bitset64 bs2(std::move(bs1));
    CHECK(bs2.size() == 100);
    CHECK(bs2.test(50) == true);

    // bs1 should be gutted
    CHECK(bs1.size() == 0);
    CHECK(bs1.data() == nullptr);
  }

  SUBCASE("Move Assignment") {
    Bitset64 bs2(20);
    bs2 = std::move(bs1);
    CHECK(bs2.size() == 100);
    CHECK(bs2.test(50) == true);
    CHECK(bs1.size() == 0);
  }
}

TEST_CASE("Alternative Block Sizes") {
  SUBCASE("8-bit blocks") {
    Bitset8 bs(20);  // Needs 3 blocks (24 bits total)
    CHECK(bs.num_blocks() == 3);

    bs.set(7);
    bs.set(8);  // Cross boundary

    CHECK(bs.test(7) == true);
    CHECK(bs.test(8) == true);

    bs <<= 5;
    CHECK(bs.test(12) == true);
    CHECK(bs.test(13) == true);
  }
}
TEST_SUITE_END();