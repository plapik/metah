#ifndef SRC_METAH_OPERATORS_ACCEPTANCE_STRATEGY_HPP_
#define SRC_METAH_OPERATORS_ACCEPTANCE_STRATEGY_HPP_

#include <memory>

namespace metah {
template <typename S> class AcceptanceStrategy {
  public:
    using ScoreType = S;
    [[nodiscard]] virtual bool shouldAccept(const ScoreType &current_score, const ScoreType &neighbour_score,
                                            double temperature) const = 0;
    [[nodiscard]] virtual std::unique_ptr<AcceptanceStrategy> clone() const = 0;
    virtual ~AcceptanceStrategy() = default;
};
} // namespace metah

#endif
