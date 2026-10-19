#ifndef SRC_METAH_PROBLEM_HPP_
#define SRC_METAH_PROBLEM_HPP_

#include "metah/solution.hpp"

namespace metah {
/**
 * @brief Objective function for candidate representations.
 * @tparam R Representation type.
 * @tparam S Score type compatible with the selected optimizer and operators.
 * @tparam minimizing True to prefer lower scores, false to prefer higher scores.
 *
 * Implement evaluate() for your problem. Parallel optimizers require concurrent
 * calls on the same instance to be safe.
 *
 * @note R and S must be copy-constructible and copy-assignable. Distinct
 * representations and scores must have independent storage; aliasing causes
 * undefined behavior. S must support `<`, `<=`, `==`, `!=`, `>=`, and `>`.
 */
template <typename R, typename S, bool minimizing> class Problem {
  public:
    using ReprType = R;
    using ScoreType = S;
    using SolutionType = Solution<ReprType, ScoreType>;
    static constexpr bool isMinimizing = minimizing;

    /// @brief Scores a candidate that satisfies the concrete problem's constraints.
    [[nodiscard]] virtual ScoreType evaluate(const ReprType &repr) const = 0;

    virtual ~Problem() = default;
};
} // namespace metah

#endif // SRC_METAH_PROBLEM_HPP_
