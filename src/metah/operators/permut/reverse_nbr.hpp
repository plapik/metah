#ifndef SRC_METAH_OPERATORS_PERMUT_REVERSE_NBR_HPP_
#define SRC_METAH_OPERATORS_PERMUT_REVERSE_NBR_HPP_

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
#include <memory>

#include "metah/operators/neighbour_gen.hpp"
#include "metah/util/rng.hpp"

namespace metah::permut {
struct ReverseMove {
  size_t i;
  size_t j;

  friend bool operator==(const ReverseMove&, const ReverseMove&) = default;

  template <typename H>
  friend H AbslHashValue(H h, const ReverseMove& move) {
    return H::combine(std::move(h), move.i, move.j);
  }
};

template <typename R = std::vector<size_t>, typename RNG = ThreadLocalRandom>
class ReverseNeighbour final : public NeighbourGen<R, ReverseMove> {
  [[no_unique_address]] RNG rng_;
  size_t nbr_size_;

 public:
  using Base = NeighbourGen<R, ReverseMove>;
  using typename Base::MoveType;
  using typename Base::ReprType;

  explicit ReverseNeighbour(const size_t neighbourhood_size, RNG rng = RNG())
      : rng_(std::move(rng)), nbr_size_(neighbourhood_size) {}

  [[nodiscard]] size_t size(const ReprType& original) const override {
    return nbr_size_;
  }

  MoveType generate(const ReprType& original, ReprType& target,
                    size_t idx) override {
    assert(&original != &target);
    assert(std::size(target) >= 2);
    assert(std::size(target) == std::size(original));
    auto [r1, r2] = rng_.randomPair(std::size(original) - 1);
    const auto idx1 = std::min(r1, r2);
    const auto idx2 = std::max(r1, r2);
    std::copy(std::begin(original), std::begin(original) + idx1,
              std::begin(target));
    std::reverse_copy(std::begin(original) + idx1,
                      std::begin(original) + idx2 + 1,
                      std::begin(target) + idx1);
    std::copy(std::begin(original) + idx2 + 1, std::end(original),
              std::begin(target) + idx2 + 1);
    return {idx1, idx2};
  }

  std::unique_ptr<NeighbourGen<ReprType, MoveType>> clone() const override {
    return std::make_unique<ReverseNeighbour>(*this);
  }
};
}  // namespace metah::permut

#endif  // SRC_METAH_OPERATORS_PERMUT_REVERSE_NBR_HPP_
