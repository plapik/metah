#ifndef SRC_METAH_OPERATORS_ACCEPTANCE_STRATEGY_HPP_
#define SRC_METAH_OPERATORS_ACCEPTANCE_STRATEGY_HPP_

#include <memory>

namespace metah {
/// @brief Decides whether simulated annealing accepts a candidate score at a given temperature.
template <typename S> class AcceptanceStrategy {
  public:
    using ScoreType = S;
    /// @brief Returns whether the candidate should replace the current solution.
    [[nodiscard]] virtual bool shouldAccept(const ScoreType &current_score, const ScoreType &neighbour_score,
                                            double temperature) const = 0;
    /// @brief Returns an owning copy of this operator.
    [[nodiscard]] virtual std::unique_ptr<AcceptanceStrategy> clone() const = 0;
    virtual ~AcceptanceStrategy() = default;
};
} // namespace metah

#endif
