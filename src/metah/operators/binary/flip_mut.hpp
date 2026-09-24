#ifndef SRC_METAH_OPERATORS_BINARY_FLIP_MUT_HPP_
#define SRC_METAH_OPERATORS_BINARY_FLIP_MUT_HPP_

#include <cassert>
#include <vector>
#include <utility>
#include <memory>

#include "metah/operators/mutate.hpp"
#include "metah/util/rng.hpp"

namespace metah::binary {
template <typename R = std::vector<uint8_t>, typename RNG = ThreadLocalRandom>
class FlipMut final : public Mutate<R> {
  [[no_unique_address]] RNG rng_;
  double chance_;

 public:
  using Base = Mutate<R>;
  using typename Base::ReprType;

  explicit FlipMut(const double chance, RNG rng = RNG())
      : rng_(std::move(rng)), chance_(chance) {}

  bool mutate(ReprType& repr) override {
    const size_t n = std::size(repr);
    assert(n >= 1);

    if (chance_ <= 0.0) return false;
    if (chance_ >= 1.0) {
      for (size_t i = 0; i < n; ++i) {
        repr[i] = repr[i] ^ 1;
      }
      return true;
    }

    bool changed = false;
    size_t i = 0;
    while (i < n) {
      const size_t gap = rng_.randomGeometricGap(chance_);
      i += gap;
      if (i >= n) break;
      repr[i] = repr[i] ^ 1;
      changed = true;
      ++i;
    }
    return changed;
  }

  std::unique_ptr<Mutate<ReprType>> clone() const override {
    return std::make_unique<FlipMut>(*this);
  }

  ~FlipMut() override = default;
};
}  // namespace metah::binary

#endif  // SRC_METAH_OPERATORS_BINARY_FLIP_MUT_HPP_
