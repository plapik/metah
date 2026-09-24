#ifndef SRC_METAH_OPERATORS_PERMUT_PMX_HPP_
#define SRC_METAH_OPERATORS_PERMUT_PMX_HPP_

#include <cassert>
#include <utility>
#include <limits>
#include <vector>
#include <algorithm>
#include <memory>

#include "metah/operators/crossover.hpp"
#include "metah/util/rng.hpp"

namespace metah::permut {
template <typename R = std::vector<size_t>, typename RNG = ThreadLocalRandom>
class Pmx final : public Crossover<R> {
  [[no_unique_address]] RNG rng_;
  double chance_;

 public:
  using Base = Crossover<R>;
  using typename Base::ReprType;
  explicit Pmx(const double chance, RNG rng = RNG())
      : rng_(std::move(rng)), chance_(chance) {}
  bool cross(const ReprType& p1, const ReprType& p2, ReprType& c1,
             ReprType& c2) override {
    assert(&p1 != &c1 && &p1 != &c2 && &p2 != &c1 && &p2 != &c2);
    assert(std::size(p1) >= 2);
    assert(std::size(p1) == std::size(p2));
    assert(std::size(p1) == std::size(c1));
    assert(std::size(c1) == std::size(c2));
    auto size = std::size(p1);

    if (!rng_.randomBool(chance_)) {
      std::copy(std::begin(p1), std::end(p1), std::begin(c1));
      std::copy(std::begin(p2), std::end(p2), std::begin(c2));
      return false;
    }

    constexpr size_t unmapped = std::numeric_limits<size_t>::max();
    thread_local std::vector<size_t> gene_idxs_c1;
    thread_local std::vector<size_t> gene_idxs_c2;
    gene_idxs_c1.assign(size, unmapped);
    gene_idxs_c2.assign(size, unmapped);

    auto [idx1, idx2] = rng_.randomPair(size - 1);
    auto begin = std::min(idx1, idx2);
    auto end = std::max(idx1, idx2) + 1;

    std::copy(std::begin(p1) + begin, std::begin(p1) + end,
              std::begin(c2) + begin);
    std::copy(std::begin(p2) + begin, std::begin(p2) + end,
              std::begin(c1) + begin);
    for (auto i = begin; i < end; ++i) {
      gene_idxs_c1[c1[i]] = i;
      gene_idxs_c2[c2[i]] = i;
    }

    for (size_t i = 0; i < begin; ++i) {
      auto val1 = p1[i];
      while (gene_idxs_c1[val1] != unmapped) {
        val1 = p1[gene_idxs_c1[val1]];
      }
      c1[i] = val1;

      auto val2 = p2[i];
      while (gene_idxs_c2[val2] != unmapped) {
        val2 = p2[gene_idxs_c2[val2]];
      }
      c2[i] = val2;
    }

    for (size_t i = end; i < size; ++i) {
      auto val1 = p1[i];
      while (gene_idxs_c1[val1] != unmapped) {
        val1 = p1[gene_idxs_c1[val1]];
      }
      c1[i] = val1;

      auto val2 = p2[i];
      while (gene_idxs_c2[val2] != unmapped) {
        val2 = p2[gene_idxs_c2[val2]];
      }
      c2[i] = val2;
    }

    return true;
  }

  [[nodiscard]] std::unique_ptr<Crossover<ReprType>> clone() const override {
    return std::make_unique<Pmx>(*this);
  }
};
}  // namespace metah::permut

#endif  // SRC_METAH_OPERATORS_PERMUT_PMX_HPP_
