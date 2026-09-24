#ifndef TEST_TEST_RNG_HPP_
#define TEST_TEST_RNG_HPP_

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

class TestRNG {
  struct MockEngine {
    TestRNG* self;
    using result_type = uint32_t;

    static constexpr result_type min() { return 0; }
    static constexpr result_type max() {
      return std::numeric_limits<result_type>::max();
    }

    result_type operator()() {
      const long double u_raw = static_cast<long double>(self->random01());
      const long double u =
          (u_raw <= 0.0L)
              ? std::nextafter(0.0L, 1.0L)
              : (u_raw >= 1.0L ? std::nextafter(1.0L, 0.0L) : u_raw);
      const long double x = u * (static_cast<long double>(max()) + 1.0L);
      return static_cast<result_type>(
          std::clamp(x, 0.0L, static_cast<long double>(max())));
    }
  };

  std::vector<size_t> idxs_;
  std::vector<double> doubles_;
  std::vector<std::pair<size_t, size_t>> pairs_;
  std::vector<size_t> offsets_;
  size_t idxs_cnt_;
  size_t offsets_cnt_;
  size_t doubles_cnt_;
  size_t pairs_cnt_;
  MockEngine engine_;

 public:
  TestRNG(std::vector<size_t> idxs, std::vector<double> doubles,
          std::vector<std::pair<size_t, size_t>> pairs,
          std::vector<size_t> offsets = {})
      : idxs_(std::move(idxs)),
        doubles_(std::move(doubles)),
        pairs_(std::move(pairs)),
        offsets_(std::move(offsets)),
        idxs_cnt_(0),
        offsets_cnt_(0),
        doubles_cnt_(0),
        pairs_cnt_(0),
        engine_{this} {}

  MockEngine& randomEngine() { return engine_; }

  size_t randomIdx(const size_t min, const size_t max) {
    if (idxs_.empty()) {
      throw std::runtime_error("Empty idx rng mock");
    }
    const auto idx = idxs_[idxs_cnt_++];
    if (idxs_cnt_ == idxs_.size()) {
      idxs_cnt_ = 0;
    }
    return idx;
  }

  size_t randomIdx(const size_t max) { return randomIdx(0, max); }

  double random01() {
    if (doubles_.empty()) {
      throw std::runtime_error("Empty 01 rng mock");
    }
    const auto value = doubles_[doubles_cnt_++];
    if (doubles_cnt_ == doubles_.size()) {
      doubles_cnt_ = 0;
    }
    return value;
  }

  bool randomBool(const double chance) { return random01() <= chance; }

  size_t randomGeometricGap(const double /*chance*/) {
    if (offsets_.empty()) {
      throw std::runtime_error("Empty geometric gap rng mock");
    }
    const auto gap = offsets_[offsets_cnt_++];
    if (offsets_cnt_ == offsets_.size()) {
      offsets_cnt_ = 0;
    }
    return gap;
  }

  std::pair<size_t, size_t> randomPair(const size_t max) {
    if (pairs_.empty()) {
      throw std::runtime_error("Empty pair rng mock");
    }
    const auto pair = pairs_[pairs_cnt_++];
    if (pairs_cnt_ == pairs_.size()) {
      pairs_cnt_ = 0;
    }
    return pair;
  }
};

#endif  // TEST_TEST_RNG_HPP_
