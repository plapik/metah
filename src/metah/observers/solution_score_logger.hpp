#ifndef SRC_METAH_OBSERVERS_SOLUTION_SCORE_LOGGER_HPP_
#define SRC_METAH_OBSERVERS_SOLUTION_SCORE_LOGGER_HPP_

#include <fmt/base.h>
#include <fmt/os.h>

#include "metah/observer.hpp"

namespace metah {
/**
 * @brief Writes solution scores as iteration,score CSV rows.
 * @tparam R Solution representation type; representations are not inspected.
 * @tparam S Score type supporting formatting with {fmt}.
 * @note Owns a buffered file stream. Concurrent updates require external synchronization.
 */
template <typename R, typename S> class SolutionScoreFileLogger final : public SolutionObserver<R, S> {
  public:
    using Base = SolutionObserver<R, S>;
    using typename Base::ReprType;
    using typename Base::ScoreType;

    /// @brief Creates or truncates filepath and writes the CSV header.
    /// @throws std::system_error if the file cannot be opened.
    explicit SolutionScoreFileLogger(const char *filepath) : out_(fmt::output_file(filepath)) {
        out_.print("iteration,score\n");
    }

    /// @brief Appends the iteration and score; repr is ignored.
    void update(size_t iteration, const ReprType &repr, const ScoreType &score) override {
        out_.print("{},{}\n", iteration, score);
    }

  private:
    fmt::ostream out_;
};
} // namespace metah

#endif // SRC_METAH_OBSERVERS_SOLUTION_SCORE_LOGGER_HPP_
