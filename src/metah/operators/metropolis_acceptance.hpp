#ifndef SRC_METAH_OPERATORS_METROPOLIS_ACCEPTANCE_HPP_
#define SRC_METAH_OPERATORS_METROPOLIS_ACCEPTANCE_HPP_

#include <cmath>
#include <memory>
#include <utility>

#include "metah/operators/acceptance_strategy.hpp"
#include "metah/util/rng.hpp"

namespace metah {
template <typename S, bool isMinimizing, typename RNG = ThreadLocalRandom>
class MetropolisAcceptance final : public AcceptanceStrategy<S> {
    [[no_unique_address]] RNG rng_;

  public:
    using ScoreType = S;
    explicit MetropolisAcceptance(RNG rng = RNG()) : rng_(std::move(rng)) {}

    [[nodiscard]] bool shouldAccept(const ScoreType &current_score, const ScoreType &neighbour_score,
                                    const double temperature) const override {
        if (temperature <= 0.0) {
            return false;
        }

        double delta;
        if constexpr (isMinimizing) {
            delta = static_cast<double>(current_score) - static_cast<double>(neighbour_score);
        } else {
            delta = static_cast<double>(neighbour_score) - static_cast<double>(current_score);
        }

        return std::exp(delta / temperature) > rng_.random01();
    }

    [[nodiscard]] std::unique_ptr<AcceptanceStrategy<S>> clone() const override {
        return std::make_unique<MetropolisAcceptance>(*this);
    }
};
} // namespace metah

#endif
