#include <vector>
#include <benchmark/benchmark.h>

#include <metah/operators/binary/bitset_single_point_cross.hpp>
#include <metah/operators/binary/single_point_cross.hpp>

#include "common.hpp"

static void BM_SinglePointCrossover(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  const auto p1 = bench::operators::make_binary_vector(size);
  const auto p2 = bench::operators::make_binary_vector(size);
  std::vector<uint8_t> c1(size);
  std::vector<uint8_t> c2(size);
  metah::binary::SinglePointCrossover<> crossover(bench::operators::k_always);

  for (auto _ : state) {
    benchmark::DoNotOptimize(crossover.cross(p1, p2, c1, c2));
    benchmark::DoNotOptimize(c1);
    benchmark::DoNotOptimize(c2);
  }
}

static void BM_BitsetSinglePointCrossover(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  const auto p1 = bench::operators::make_bitset(size);
  const auto p2 = bench::operators::make_bitset(size);
  metah::DynamicBitset<> c1(size);
  metah::DynamicBitset<> c2(size);
  metah::binary::BitsetSinglePointCrossover<> crossover(
      bench::operators::k_always);

  for (auto _ : state) {
    benchmark::DoNotOptimize(crossover.cross(p1, p2, c1, c2));
    benchmark::DoNotOptimize(c1);
    benchmark::DoNotOptimize(c2);
  }
}

BENCHMARK(BM_SinglePointCrossover)->RangeMultiplier(4)->Range(16, 65536);
BENCHMARK(BM_BitsetSinglePointCrossover)->RangeMultiplier(4)->Range(16, 65536);
