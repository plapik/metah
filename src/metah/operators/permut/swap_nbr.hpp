#ifndef SRC_METAH_OPERATORS_PERMUT_SWAP_NBR_HPP_
#define SRC_METAH_OPERATORS_PERMUT_SWAP_NBR_HPP_

#include <cassert>
#include <utility>
#include <vector>
#include <memory>

#include "metah/operators/neighbour_gen.hpp"
#include "metah/util/rng.hpp"

namespace metah::permut {
struct SwapMove {
  size_t i;
  size_t j;

  friend bool operator==(const SwapMove&, const SwapMove&) = default;

  template <typename H>
  friend H AbslHashValue(H h, const SwapMove& move) {
    const auto lo = move.i < move.j ? move.i : move.j;
    const auto hi = move.i < move.j ? move.j : move.i;
    return H::combine(std::move(h), lo, hi);
  }
};

template <typename R = std::vector<size_t>, typename RNG = ThreadLocalRandom>
class RngSwapNeighbour final : public NeighbourGen<R, SwapMove> {
  [[no_unique_address]] RNG rng_;
  size_t nbr_size_;

 public:
  using Base = NeighbourGen<R, SwapMove>;
  using typename Base::MoveType;
  using typename Base::ReprType;

  explicit RngSwapNeighbour(const size_t neighbourhood_size, RNG rng = RNG())
      : rng_(std::move(rng)), nbr_size_(neighbourhood_size) {}

  [[nodiscard]] size_t size(const ReprType& original) const override {
    return nbr_size_;
  }

  MoveType generate(const ReprType& original, ReprType& target,
                    size_t idx) override {
    assert(&original != &target);
    assert(std::size(original) >= 2);
    target = original;
    auto [idx1, idx2] = rng_.randomPair(std::size(target) - 1);
    using std::swap;
    swap(target[idx1], target[idx2]);
    return {idx1, idx2};
  }

  [[nodiscard]] std::unique_ptr<NeighbourGen<ReprType, MoveType>> clone()
      const override {
    return std::make_unique<RngSwapNeighbour>(*this);
  }
};
}  // namespace metah::permut

#endif  // SRC_METAH_OPERATORS_PERMUT_SWAP_NBR_HPP_
