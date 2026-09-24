#ifndef SRC_METAH_OBSERVER_HPP_
#define SRC_METAH_OBSERVER_HPP_

#include <span>

#include "metah/solution.hpp"

namespace metah {
template <typename R, typename S>
class SolutionObserver {
 public:
  using ReprType = R;
  using ScoreType = S;
  virtual void update(size_t iteration, const ReprType& repr,
                      const ScoreType& score) = 0;

  void update(const size_t iteration,
              const Solution<ReprType, ScoreType>& solution) {
    update(iteration, solution.repr, solution.score);
  }

  virtual ~SolutionObserver() = default;
};

template <typename R, typename S>
class PopulationObserver {
 public:
  using ReprType = R;
  using ScoreType = S;
  virtual void update(size_t iteration, std::span<const ReprType> population,
                      std::span<const ScoreType> scores) = 0;
  virtual ~PopulationObserver() = default;
};
}  // namespace metah

#endif  // SRC_METAH_OBSERVER_HPP_
