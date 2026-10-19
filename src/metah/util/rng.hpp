#ifndef SRC_METAH_UTIL_RNG_HPP_
#define SRC_METAH_UTIL_RNG_HPP_

#include <random>
#include <utility>

#include <boost/random/xoshiro.hpp>

namespace metah {
/**
 * @brief Random sampling with one lazily seeded engine per thread.
 * @note Thread-safe: each thread uses its own engine, shared by all instances
 * on that thread. Do not share engine references across threads.
 */
class ThreadLocalRandom {
  public:
    ThreadLocalRandom() = default;

    /// @brief Samples a uniform index in [min, max]. Requires min <= max.
    static size_t randomIdx(const size_t min, const size_t max) {
        std::uniform_int_distribution dist(min, max);
        return dist(randomEngine());
    }

    /// @brief Samples a uniform index in [0, max].
    static size_t randomIdx(const size_t max) { return randomIdx(0, max); }

    /// @brief Samples two distinct indices in [0, max]. Requires max >= 1.
    static std::pair<size_t, size_t> randomPair(const size_t max) {
        const auto idx1 = randomIdx(max);
        auto idx2 = randomIdx(max - 1);
        idx2 += idx2 >= idx1;
        return {idx1, idx2};
    }

    /// @brief Samples a uniform real value in [0, 1).
    static double random01() {
        std::uniform_real_distribution dist(0.0, 1.0);
        return dist(randomEngine());
    }

    /// @brief Returns true with the given probability. Requires chance in [0, 1].
    static bool randomBool(const double chance) {
        std::bernoulli_distribution dist(chance);
        return dist(randomEngine());
    }

    /// @brief Samples failures before the first success. Requires chance in (0, 1).
    static size_t randomGeometricGap(const double chance) {
        std::geometric_distribution<size_t> dist(chance);
        return dist(randomEngine());
    }

    /// @brief Returns this thread's engine, seeded from std::random_device on first use.
    static boost::random::xoshiro256pp &randomEngine() {
        thread_local boost::random::xoshiro256pp rng(std::random_device{}());
        return rng;
    }
};

/**
 * @brief Random sampling with one lazily seeded engine shared by all instances.
 * @warning Not thread-safe. Concurrent calls require external synchronization,
 * including direct access through randomEngine().
 */
class GlobalRandom {
  public:
    GlobalRandom() = default;

    /// @brief Samples a uniform index in [min, max]. Requires min <= max.
    static size_t randomIdx(const size_t min, const size_t max) {
        std::uniform_int_distribution dist(min, max);
        return dist(randomEngine());
    }

    /// @brief Samples a uniform index in [0, max].
    static size_t randomIdx(const size_t max) { return randomIdx(0, max); }

    /// @brief Samples two distinct indices in [0, max]. Requires max >= 1.
    static std::pair<size_t, size_t> randomPair(const size_t max) {
        const auto idx1 = randomIdx(max);
        auto idx2 = randomIdx(max - 1);
        idx2 += idx2 >= idx1;
        return {idx1, idx2};
    }

    /// @brief Samples a uniform real value in [0, 1).
    static double random01() {
        std::uniform_real_distribution dist(0.0, 1.0);
        return dist(randomEngine());
    }

    /// @brief Returns true with the given probability. Requires chance in [0, 1].
    static bool randomBool(const double chance) {
        std::bernoulli_distribution dist(chance);
        return dist(randomEngine());
    }

    /// @brief Samples failures before the first success. Requires chance in (0, 1).
    static size_t randomGeometricGap(const double chance) {
        std::geometric_distribution<size_t> dist(chance);
        return dist(randomEngine());
    }

    /// @brief Returns the shared engine, seeded from std::random_device on first use.
    static boost::random::xoshiro256pp &randomEngine() {
        static boost::random::xoshiro256pp global_rng(std::random_device{}());
        return global_rng;
    }
};
} // namespace metah

#endif // SRC_METAH_UTIL_RNG_HPP_
