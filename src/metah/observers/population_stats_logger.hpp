#ifndef SRC_METAH_OBSERVERS_POPULATION_STATS_LOGGER_HPP_
#define SRC_METAH_OBSERVERS_POPULATION_STATS_LOGGER_HPP_

#include <fmt/base.h>
#include <fmt/os.h>
#include <numeric>

#include "metah/observer.hpp"

namespace metah {
/**
 * @brief Writes population statistics as iteration,min,max,avg CSV rows.
 * @tparam R Population representation type; representations are not inspected.
 * @tparam S Score type supporting ordering, zero construction, addition, division
 * by size_t, and formatting with {fmt}.
 * @note Average uses score-type accumulation and an unsigned divisor. Integral
 * averages may truncate, and negative integral totals may convert to unsigned.
 * @note Owns a buffered file stream. Concurrent updates require external synchronization.
 */
template <typename R, typename S> class PopulationStatsFileLogger final : public PopulationObserver<R, S> {
  public:
    using Base = PopulationObserver<R, S>;
    using typename Base::ReprType;
    using typename Base::ScoreType;

    /// @brief Creates or truncates filepath and writes the CSV header.
    /// @throws std::system_error if the file cannot be opened.
    explicit PopulationStatsFileLogger(const char *filepath) : out_(fmt::output_file(filepath)) {
        out_.print("iteration,min,max,avg\n");
    }

    /// @brief Appends statistics for scores, or '-' fields if empty; population is ignored.
    void update(size_t iteration, std::span<const ReprType> population, std::span<const ScoreType> scores) override {
        if (scores.empty()) [[unlikely]] {
            out_.print("{},-,-,-\n", iteration);
            return;
        }

        auto max = *std::ranges::max_element(scores);
        auto min = *std::ranges::min_element(scores);
        auto avg = std::accumulate(scores.begin(), scores.end(), ScoreType(0)) / scores.size();
        out_.print("{},{},{},{}\n", iteration, min, max, avg);
    }

  private:
    fmt::ostream out_;
};
} // namespace metah

#endif // SRC_METAH_OBSERVERS_POPULATION_STATS_LOGGER_HPP_
