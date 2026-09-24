#ifndef SRC_METAH_OPERATORS_PERMUT_REVERSE_PERTURBATION_HPP_
#define SRC_METAH_OPERATORS_PERMUT_REVERSE_PERTURBATION_HPP_

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
#include <memory>

#include "metah/operators/perturbation.hpp"
#include "metah/util/rng.hpp"

namespace metah::permut {
template <typename R = std::vector<size_t>, typename RNG = ThreadLocalRandom>
class ReversePerturbation final : public Perturbation<R> {
  [[no_unique_address]] RNG rng_;

 public:
  using Base = Perturbation<R>;
  using typename Base::ReprType;

  explicit ReversePerturbation(RNG rng = RNG()) : rng_(std::move(rng)) {}

  void perturb(const ReprType& current, ReprType& target) override {
    assert(std::size(current) >= 2);
    target = current;
    auto [r1, r2] = rng_.randomPair(std::size(target) - 1);
    const auto idx1 = std::min(r1, r2);
    const auto idx2 = std::max(r1, r2);
    std::reverse(std::begin(target) + idx1, std::begin(target) + idx2 + 1);
  }

  [[nodiscard]] std::unique_ptr<Perturbation<ReprType>> clone() const override {
    return std::make_unique<ReversePerturbation>(*this);
  }
};
}  // namespace metah::permut

#endif  // SRC_METAH_OPERATORS_PERMUT_REVERSE_PERTURBATION_HPP_
