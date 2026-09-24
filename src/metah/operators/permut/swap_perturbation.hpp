#ifndef SRC_METAH_OPERATORS_PERMUT_SWAP_PERTURBATION_HPP_
#define SRC_METAH_OPERATORS_PERMUT_SWAP_PERTURBATION_HPP_

#include <cassert>
#include <utility>
#include <vector>
#include <memory>

#include "metah/operators/perturbation.hpp"
#include "metah/util/rng.hpp"

namespace metah::permut {
template <typename R = std::vector<size_t>, typename RNG = ThreadLocalRandom>
class SwapPerturbation final : public Perturbation<R> {
  [[no_unique_address]] RNG rng_;

 public:
  using Base = Perturbation<R>;
  using typename Base::ReprType;

  explicit SwapPerturbation(RNG rng = RNG()) : rng_(std::move(rng)) {}

  void perturb(const ReprType& current, ReprType& target) override {
    assert(std::size(current) >= 2);
    target = current;
    auto [idx1, idx2] = rng_.randomPair(std::size(target) - 1);
    using std::swap;
    swap(target[idx1], target[idx2]);
  }

  [[nodiscard]] std::unique_ptr<Perturbation<ReprType>> clone() const override {
    return std::make_unique<SwapPerturbation>(*this);
  }
};
}  // namespace metah::permut

#endif  // SRC_METAH_OPERATORS_PERMUT_SWAP_PERTURBATION_HPP_
