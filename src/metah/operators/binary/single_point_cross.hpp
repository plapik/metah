#ifndef SRC_METAH_OPERATORS_BINARY_SINGLE_POINT_CROSS_HPP_
#define SRC_METAH_OPERATORS_BINARY_SINGLE_POINT_CROSS_HPP_

#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include "metah/operators/crossover.hpp"
#include "metah/util/rng.hpp"

namespace metah::binary {
/**
 * @brief Produces offspring by exchanging parent tails at one random split point.
 * @ingroup binary_operators
 */
template <typename R = std::vector<uint8_t>, typename RNG = ThreadLocalRandom>
class SinglePointCrossover final : public Crossover<R> {
    [[no_unique_address]] RNG rng_;
    double chance_;

  public:
    using Base = Crossover<R>;
    using typename Base::ReprType;

    /// @brief Creates a single-point crossover with probability chance in [0, 1].
    explicit SinglePointCrossover(const double chance, RNG rng = RNG()) : rng_(std::move(rng)), chance_(chance) {}

    /// @brief Writes offspring; copies corresponding parents when skipped and returns false.
    bool cross(const ReprType &p1, const ReprType &p2, ReprType &c1, ReprType &c2) override {
        assert(std::size(p1) >= 2);
        assert(std::size(p1) == std::size(p2));
        assert(std::size(p1) == std::size(c1));
        assert(std::size(c1) == std::size(c2));
        if (!rng_.randomBool(chance_)) {
            c1 = p1;
            c2 = p2;
            return false;
        }

        const size_t pt = rng_.randomIdx(1, std::size(p1) - 1);
        std::copy_n(std::begin(p1), pt, std::begin(c1));
        std::copy_n(std::begin(p2), pt, std::begin(c2));
        std::copy(std::begin(p2) + pt, std::end(p2), std::begin(c1) + pt);
        std::copy(std::begin(p1) + pt, std::end(p1), std::begin(c2) + pt);

        return true;
    }

    [[nodiscard]] std::unique_ptr<Crossover<ReprType>> clone() const override {
        return std::make_unique<SinglePointCrossover>(*this);
    }
};
} // namespace metah::binary

#endif // SRC_METAH_OPERATORS_BINARY_SINGLE_POINT_CROSS_HPP_
