#ifndef BENCHMARK_MICROBENCHMARKS_OPERATORS_COMMON_HPP
#define BENCHMARK_MICROBENCHMARKS_OPERATORS_COMMON_HPP

#include <numeric>
#include <vector>
#include <metah/util/dynamic_bitset.hpp>


namespace bench::operators {

inline std::vector<size_t> make_permutation(const size_t n) {
  std::vector<size_t> repr(n);
  std::iota(repr.begin(), repr.end(), 0);
  return repr;
}

inline std::vector<uint8_t> make_binary_vector(const size_t n) {
  std::vector<uint8_t> repr(n);
  for (size_t i = 0; i < n; ++i) {
    repr[i] = static_cast<uint8_t>(i & 1);
  }
  return repr;
}

inline metah::DynamicBitset<> make_bitset(const size_t n) {
  metah::DynamicBitset<> repr(n);
  for (size_t i = 0; i < n; ++i) {
    if (i & 1) {
      repr.set(i);
    } else {
      repr.reset(i);
    }
  }
  return repr;
}

inline std::vector<int64_t> make_scores(const size_t n) {
  std::vector<int64_t> scores(n);
  for (size_t i = 0; i < n; ++i) {
    scores[i] = static_cast<int64_t>((i * 17 + 3) % 1000);
  }
  return scores;
}

constexpr double k_always = 1.0;

inline double flip_chance(const size_t solution_size) {
  return 1.0 / static_cast<double>(solution_size);
}

}  // namespace bench::operators

#endif  // BENCHMARK_MICROBENCHMARKS_OPERATORS_COMMON_HPP
