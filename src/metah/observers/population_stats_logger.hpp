#ifndef SRC_METAH_OBSERVERS_POPULATION_STATS_LOGGER_HPP_
#define SRC_METAH_OBSERVERS_POPULATION_STATS_LOGGER_HPP_

#include <numeric>
#include <fmt/base.h>
#include <fmt/os.h>
#include "metah/observer.hpp"

namespace metah {
template <typename R, typename S>
class PopulationStatsFileLogger final : public PopulationObserver<R, S> {
 public:
  using Base = PopulationObserver<R, S>;
  using typename Base::ReprType;
  using typename Base::ScoreType;

  explicit PopulationStatsFileLogger(const char* filepath)
      : out_(fmt::output_file(filepath)) {
    out_.print("iteration,min,max,avg\n");
  }

  void update(size_t iteration, std::span<const ReprType> population,
              std::span<const ScoreType> scores) override {
    if (scores.empty()) [[unlikely]] {
      out_.print("{},-,-,-\n", iteration);
      return;
    }

    auto max = *std::ranges::max_element(scores);
    auto min = *std::ranges::min_element(scores);
    auto avg = std::accumulate(scores.begin(), scores.end(), ScoreType(0)) /
               scores.size();
    out_.print("{},{},{},{}\n", iteration, min, max, avg);
  }

 private:
  fmt::ostream out_;
};
}  // namespace metah

#endif  // SRC_METAH_OBSERVERS_POPULATION_STATS_LOGGER_HPP_
