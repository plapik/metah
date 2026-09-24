#ifndef SRC_METAH_OPERATORS_BINARY_BITSET_SINGLE_POINT_CROSS_HPP_
#define SRC_METAH_OPERATORS_BINARY_BITSET_SINGLE_POINT_CROSS_HPP_

#include <algorithm>
#include <cassert>
#include <type_traits>
#include <memory>
#include <utility>

#include "metah/operators/crossover.hpp"
#include "metah/util/dynamic_bitset.hpp"
#include "metah/util/rng.hpp"

namespace metah::binary {
template <typename R = DynamicBitset<>, typename RNG = ThreadLocalRandom>
class BitsetSinglePointCrossover final : public Crossover<R> {
  [[no_unique_address]] RNG rng_;
  double chance_;

  using Block = std::remove_pointer_t<decltype(std::declval<R&>().data())>;

 public:
  using Base = Crossover<R>;
  using typename Base::ReprType;

  explicit BitsetSinglePointCrossover(const double chance, RNG rng = RNG())
      : rng_(std::move(rng)), chance_(chance) {}

  bool cross(const ReprType& p1, const ReprType& p2, ReprType& c1,
             ReprType& c2) override {
    assert(&p1 != &c1 && &p1 != &c2 && &p2 != &c1 && &p2 != &c2);
    assert(std::size(p1) >= 2);
    assert(std::size(p1) == std::size(p2));
    assert(std::size(p1) == std::size(c1));
    assert(std::size(c1) == std::size(c2));

    if (!rng_.randomBool(chance_)) {
      return false;
    }

    const size_t pt = rng_.randomIdx(1, std::size(p1) - 1);
    const size_t b_idx = ReprType::blockIndex(pt);
    const size_t bit_idx = ReprType::bitIndex(pt);
    const size_t num_blocks = p1.num_blocks();

    const Block* p1_data = p1.data();
    const Block* p2_data = p2.data();
    Block* c1_data = c1.data();
    Block* c2_data = c2.data();

    std::copy_n(p1_data, b_idx + 1, c1_data);
    std::copy_n(p2_data, b_idx + 1, c2_data);
    Block diff =
        (p1_data[b_idx] ^ p2_data[b_idx]) & ~((Block(1) << bit_idx) - 1);
    c1_data[b_idx] ^= diff;
    c2_data[b_idx] ^= diff;

    std::copy_n(p2_data + b_idx + 1, num_blocks - b_idx - 1,
                c1_data + b_idx + 1);
    std::copy_n(p1_data + b_idx + 1, num_blocks - b_idx - 1,
                c2_data + b_idx + 1);

    return true;
  }

  [[nodiscard]] std::unique_ptr<Crossover<ReprType>> clone() const override {
    return std::make_unique<BitsetSinglePointCrossover>(*this);
  }
};
}  // namespace metah::binary

#endif  // SRC_METAH_OPERATORS_BINARY_BITSET_SINGLE_POINT_CROSS_HPP_
