#ifndef SRC_METAH_OBSERVERS_SOLUTION_SCORE_LOGGER_HPP_
#define SRC_METAH_OBSERVERS_SOLUTION_SCORE_LOGGER_HPP_

#include <fmt/base.h>
#include <fmt/os.h>

#include "metah/observer.hpp"

namespace metah {
template <typename R, typename S>
class SolutionScoreFileLogger final : public SolutionObserver<R, S> {
 public:
  using Base = SolutionObserver<R, S>;
  using typename Base::ReprType;
  using typename Base::ScoreType;

  explicit SolutionScoreFileLogger(const char* filepath)
      : out_(fmt::output_file(filepath)) {
    out_.print("iteration,score\n");
  }

  void update(size_t iteration, const ReprType& repr,
              const ScoreType& score) override {
    out_.print("{},{}\n", iteration, score);
  }

 private:
  fmt::ostream out_;
};
}  // namespace metah

#endif  // SRC_METAH_OBSERVERS_SOLUTION_SCORE_LOGGER_HPP_
