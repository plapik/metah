#ifndef SRC_METAH_OPERATORS_BINARY_FLIP_PERTURBATION_HPP_
#define SRC_METAH_OPERATORS_BINARY_FLIP_PERTURBATION_HPP_

#include <cassert>
#include <vector>
#include <utility>
#include <memory>

#include "metah/operators/perturbation.hpp"
#include "metah/util/rng.hpp"

namespace metah::binary {
template <typename R = std::vector<uint8_t>, typename RNG = ThreadLocalRandom>
class SingleFlipPerturbation final : public Perturbation<R> {
  [[no_unique_address]] RNG rng_;

 public:
  using Base = Perturbation<R>;
  using typename Base::ReprType;

  explicit SingleFlipPerturbation(RNG rng = RNG()) : rng_(std::move(rng)) {}

  void perturb(const ReprType& current, ReprType& target) override {
    assert(!current.empty());
    target = current;
    const size_t bit_index = rng_.randomIdx(std::size(target) - 1);
    target[bit_index] = target[bit_index] ^ 1;
  }

  [[nodiscard]] std::unique_ptr<Perturbation<ReprType>> clone() const override {
    return std::make_unique<SingleFlipPerturbation>(*this);
  }
};

template <typename R = std::vector<uint8_t>, typename RNG = ThreadLocalRandom>
class FlipPerturbation final : public Perturbation<R> {
  [[no_unique_address]] RNG rng_;
  double flip_chance_;

 public:
  using Base = Perturbation<R>;
  using typename Base::ReprType;

  explicit FlipPerturbation(const double flip_chance, RNG rng = RNG())
      : rng_(std::move(rng)), flip_chance_(flip_chance) {}

  void perturb(const ReprType& current, ReprType& target) override {
    assert(!current.empty());
    target = current;
    const size_t n = std::size(target);

    if (flip_chance_ <= 0.0) {
      return;
    }

    if (flip_chance_ >= 1.0) {
      for (size_t i = 0; i < n; ++i) {
        target[i] = target[i] ^ 1;
      }
      return;
    }

    size_t i = 0;
    while (i < n) {
      const size_t gap = rng_.randomGeometricGap(flip_chance_);
      i += gap;
      if (i >= n) break;
      target[i] = target[i] ^ 1;
      ++i;
    }
  }

  [[nodiscard]] std::unique_ptr<Perturbation<ReprType>> clone() const override {
    return std::make_unique<FlipPerturbation>(*this);
  }
};
}  // namespace metah::binary

#endif  // SRC_METAH_OPERATORS_BINARY_FLIP_PERTURBATION_HPP_
