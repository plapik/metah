#ifndef SRC_METAH_OPERATORS_TOURNAMENT_HPP_
#define SRC_METAH_OPERATORS_TOURNAMENT_HPP_

#include <cassert>
#include <span>
#include <memory>
#include <utility>

#include "metah/util/rng.hpp"
#include "metah/operators/selection.hpp"

namespace metah {
template <typename S, bool minimizing, typename RNG = ThreadLocalRandom>
class Tournament final : public Selection<S> {
  [[no_unique_address]] RNG rng_;
  size_t size_;

 public:
  using Base = Selection<S>;
  using typename Base::ScoreType;

  explicit Tournament(const size_t size, RNG rng = RNG())
      : rng_(std::move(rng)), size_(size) {}

  size_t select(std::span<const ScoreType> scores) override {
    assert(!scores.empty());
    const auto max_idx = scores.size() - 1;
    size_t best_idx = rng_.randomIdx(max_idx);
    for (size_t i = 1; i < size_; ++i) {
      const auto idx = rng_.randomIdx(max_idx);
      if constexpr (minimizing) {
        if (scores[idx] < scores[best_idx]) best_idx = idx;
      } else {
        if (scores[idx] > scores[best_idx]) best_idx = idx;
      }
    }
    return best_idx;
  }

  [[nodiscard]] std::unique_ptr<Selection<ScoreType>> clone() const override {
    return std::make_unique<Tournament>(*this);
  }
};
}  // namespace metah

#endif  // SRC_METAH_OPERATORS_TOURNAMENT_HPP_
