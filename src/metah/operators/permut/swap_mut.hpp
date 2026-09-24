#ifndef SRC_METAH_OPERATORS_PERMUT_SWAP_MUT_HPP_
#define SRC_METAH_OPERATORS_PERMUT_SWAP_MUT_HPP_
#include <cassert>
#include <vector>
#include <utility>
#include <memory>

#include "metah/operators/mutate.hpp"
#include "metah/util/rng.hpp"

namespace metah::permut {
template <typename R = std::vector<size_t>, typename RNG = ThreadLocalRandom>
class Swap final : public Mutate<R> {
  [[no_unique_address]] RNG rng_;
  double chance_;

 public:
  using Base = Mutate<R>;
  using typename Base::ReprType;

  explicit Swap(const double chance, RNG rng = RNG())
      : rng_(std::move(rng)), chance_(chance) {
    if (chance <= 0 || chance > 1) {
      throw std::invalid_argument("Chance must be in (0, 1]");
    }
  }

  bool mutate(ReprType& repr) override {
    assert(std::size(repr) >= 2);
    if (!rng_.randomBool(chance_)) {
      return false;
    }

    auto [idx1, idx2] = rng_.randomPair(std::size(repr) - 1);
    std::swap(repr[idx1], repr[idx2]);

    return true;
  }

  std::unique_ptr<Mutate<ReprType>> clone() const override {
    return std::make_unique<Swap>(*this);
  }

  ~Swap() override = default;
};
}  // namespace metah::permut
#endif  // SRC_METAH_OPERATORS_PERMUT_SWAP_MUT_HPP_
