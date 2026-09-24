#ifndef SRC_METAH_OPERATORS_PERMUT_OX_HPP_
#define SRC_METAH_OPERATORS_PERMUT_OX_HPP_
#include <cassert>
#include <vector>
#include <utility>
#include <algorithm>
#include <memory>

#include "metah/operators/crossover.hpp"
#include "metah/util/rng.hpp"

namespace metah::permut {
template <typename R = std::vector<size_t>, typename RNG = ThreadLocalRandom>
class Ox final : public Crossover<R> {
  [[no_unique_address]] RNG rng_;
  double chance_;

 public:
  using Base = Crossover<R>;
  using typename Base::ReprType;

  explicit Ox(const double chance, RNG rng = RNG())
      : rng_(std::move(rng)), chance_(chance) {}

  bool cross(const ReprType& p1, const ReprType& p2, ReprType& c1,
             ReprType& c2) override {
    assert(&p1 != &c1 && &p1 != &c2 && &p2 != &c1 && &p2 != &c2);
    assert(std::size(p1) >= 2);
    assert(std::size(p1) == std::size(p2));
    assert(std::size(p1) == std::size(c1));
    assert(std::size(c1) == std::size(c2));

    if (!rng_.randomBool(chance_)) {
      c1 = p1;
      c2 = p2;
      return false;
    }

    const auto size = std::size(p1);
    thread_local std::vector<uint8_t> exists1;
    thread_local std::vector<uint8_t> exists2;

    exists1.assign(size, 0);
    exists2.assign(size, 0);

    auto [rng1, rng2] = rng_.randomPair(std::size(p1) - 1);
    auto begin = std::min(rng1, rng2);
    auto end = std::max(rng1, rng2) + 1;

    std::copy(std::begin(p1) + begin, std::begin(p1) + end,
              std::begin(c1) + begin);
    std::copy(std::begin(p2) + begin, std::begin(p2) + end,
              std::begin(c2) + begin);

    for (size_t i = begin; i < end; ++i) {
      exists1[c1[i]] = 1;
      exists2[c2[i]] = 1;
    }

    size_t idx1 = end >= size ? 0 : end;
    size_t idx2 = idx1;

    for (size_t i = end; i < size; ++i) {
      while (exists1[p2[idx1]]) {
        if (++idx1 >= size) idx1 = 0;
      }
      c1[i] = p2[idx1];
      if (++idx1 >= size) idx1 = 0;

      while (exists2[p1[idx2]]) {
        if (++idx2 >= size) idx2 = 0;
      }
      c2[i] = p1[idx2];
      if (++idx2 >= size) idx2 = 0;
    }

    for (size_t i = 0; i < begin; ++i) {
      while (exists1[p2[idx1]]) {
        if (++idx1 >= size) idx1 = 0;
      }
      c1[i] = p2[idx1];
      if (++idx1 >= size) idx1 = 0;

      while (exists2[p1[idx2]]) {
        if (++idx2 >= size) idx2 = 0;
      }
      c2[i] = p1[idx2];
      if (++idx2 >= size) idx2 = 0;
    }

    return true;
  }

  [[nodiscard]] std::unique_ptr<Crossover<ReprType>> clone() const override {
    return std::make_unique<Ox>(*this);
  }
};
}  // namespace metah::permut
#endif  // SRC_METAH_OPERATORS_PERMUT_OX_HPP_
