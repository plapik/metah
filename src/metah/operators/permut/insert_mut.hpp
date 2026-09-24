#ifndef SRC_METAH_OPERATORS_PERMUT_INSERT_MUT_HPP_
#define SRC_METAH_OPERATORS_PERMUT_INSERT_MUT_HPP_
#include <algorithm>
#include <cassert>
#include <vector>
#include <utility>
#include <memory>

#include "metah/operators/mutate.hpp"
#include "metah/util/rng.hpp"


namespace metah::permut {
template <typename R = std::vector<size_t>, typename RNG = ThreadLocalRandom>
class Insert final : public Mutate<R> {
  [[no_unique_address]] RNG rng_;
  double chance_;

 public:
  using Base = Mutate<R>;
  using typename Base::ReprType;

  explicit Insert(const double chance, RNG rng = RNG())
      : rng_(std::move(rng)), chance_(chance) {
    assert(chance > 0);
    assert(chance <= 1);
  }

  bool mutate(ReprType& repr) override {
    assert(std::size(repr) >= 2);
    if (!rng_.randomBool(chance_)) {
      return false;
    }

    auto [random1, random2] = rng_.randomPair(std::size(repr) - 1);
    auto idx1 = std::min(random1, random2);
    auto idx2 = std::max(random1, random2);

    std::rotate(std::begin(repr) + idx1, std::begin(repr) + idx2,
                std::begin(repr) + idx2 + 1);

    return true;
  }

  [[nodiscard]] std::unique_ptr<Mutate<ReprType>> clone() const override {
    return std::make_unique<Insert>(*this);
  }
};
}  // namespace metah::permut

#endif  // SRC_METAH_OPERATORS_PERMUT_INSERT_MUT_HPP_
