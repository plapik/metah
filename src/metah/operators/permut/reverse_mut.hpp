#ifndef SRC_METAH_OPERATORS_PERMUT_REVERSE_MUT_HPP_
#define SRC_METAH_OPERATORS_PERMUT_REVERSE_MUT_HPP_
#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include "metah/operators/mutate.hpp"
#include "metah/util/rng.hpp"

namespace metah::permut {
/**
 * @brief Reverses the segment between two distinct random positions with a given probability.
 * @ingroup permutation_operators
 */
template <typename R = std::vector<size_t>, typename RNG = ThreadLocalRandom> class Reverse final : public Mutate<R> {
    [[no_unique_address]] RNG rng_;
    double chance_;

  public:
    using Base = Mutate<R>;
    using typename Base::ReprType;

    /// @brief Creates a reversal mutation with probability chance in (0, 1].
    /// @throws std::invalid_argument if chance <= 0 or chance > 1.
    explicit Reverse(const double chance, RNG rng = RNG()) : rng_(std::move(rng)), chance_(chance) {
        if (chance <= 0 || chance > 1) {
            throw std::invalid_argument("Chance must be in (0, 1]");
        }
    }

    /// @brief Reverses a random inclusive segment; returns whether mutation was applied.
    bool mutate(ReprType &repr) override {
        assert(std::size(repr) >= 2);
        if (!rng_.randomBool(chance_)) {
            return false;
        }

        auto [idx1, idx2] = rng_.randomPair(std::size(repr) - 1);

        std::reverse(std::begin(repr) + std::min(idx1, idx2), std::begin(repr) + std::max(idx1, idx2) + 1);

        return true;
    }

    [[nodiscard]] std::unique_ptr<Mutate<ReprType>> clone() const override { return std::make_unique<Reverse>(*this); }

    ~Reverse() override = default;
};
} // namespace metah::permut

#endif // SRC_METAH_OPERATORS_PERMUT_REVERSE_MUT_HPP_
