#ifndef SRC_METAH_PROBLEM_HPP_
#define SRC_METAH_PROBLEM_HPP_
#include "metah/solution.hpp"

namespace metah {
template <typename R, typename S, bool minimizing>
class Problem {
 public:
  using ReprType = R;
  using ScoreType = S;
  using SolutionType = Solution<ReprType, ScoreType>;
  static constexpr bool isMinimizing = minimizing;

  [[nodiscard]] virtual ScoreType evaluate(const ReprType& repr) const = 0;

  virtual ~Problem() = default;
};
}  // namespace metah

#endif  // SRC_METAH_PROBLEM_HPP_
