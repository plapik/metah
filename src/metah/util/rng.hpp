#ifndef SRC_METAH_UTIL_RNG_HPP_
#define SRC_METAH_UTIL_RNG_HPP_

#include <random>
#include <utility>
#include <boost/random/xoshiro.hpp>

namespace metah {
class ThreadLocalRandom {
 public:
  ThreadLocalRandom() = default;

  static size_t randomIdx(const size_t min, const size_t max) {
    std::uniform_int_distribution dist(min, max);
    return dist(randomEngine());
  }

  static size_t randomIdx(const size_t max) { return randomIdx(0, max); }

  static std::pair<size_t, size_t> randomPair(const size_t max) {
    const auto idx1 = randomIdx(max);
    auto idx2 = randomIdx(max - 1);
    idx2 += idx2 >= idx1;
    return {idx1, idx2};
  }

  static double random01() {
    std::uniform_real_distribution dist(0.0, 1.0);
    return dist(randomEngine());
  }

  static bool randomBool(const double chance) {
    std::bernoulli_distribution dist(chance);
    return dist(randomEngine());
  }

  static size_t randomGeometricGap(const double chance) {
    std::geometric_distribution<size_t> dist(chance);
    return dist(randomEngine());
  }

  static boost::random::xoshiro256pp& randomEngine() {
    thread_local boost::random::xoshiro256pp rng(std::random_device{}());
    return rng;
  }
};

class GlobalRandom {
 public:
  GlobalRandom() = default;

  static size_t randomIdx(const size_t min, const size_t max) {
    std::uniform_int_distribution dist(min, max);
    return dist(randomEngine());
  }

  static size_t randomIdx(const size_t max) { return randomIdx(0, max); }

  static std::pair<size_t, size_t> randomPair(const size_t max) {
    const auto idx1 = randomIdx(max);
    auto idx2 = randomIdx(max - 1);
    idx2 += idx2 >= idx1;
    return {idx1, idx2};
  }

  static double random01() {
    std::uniform_real_distribution dist(0.0, 1.0);
    return dist(randomEngine());
  }

  static bool randomBool(const double chance) {
    std::bernoulli_distribution dist(chance);
    return dist(randomEngine());
  }

  static size_t randomGeometricGap(const double chance) {
    std::geometric_distribution<size_t> dist(chance);
    return dist(randomEngine());
  }

  static boost::random::xoshiro256pp& randomEngine() {
    static boost::random::xoshiro256pp global_rng(std::random_device{}());
    return global_rng;
  }
};
}  // namespace metah

#endif  // SRC_METAH_UTIL_RNG_HPP_
