#include <benchmark/benchmark.h>

#include <metah/operators/permut/ox.hpp>
#include <metah/operators/permut/pmx.hpp>

#include "common.hpp"

static void BM_Pmx(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  const auto p1 = bench::operators::make_permutation(size);
  auto p2 = bench::operators::make_permutation(size);
  std::reverse(p2.begin(), p2.end());
  std::vector<size_t> c1(size);
  std::vector<size_t> c2(size);
  metah::permut::Pmx<> pmx(bench::operators::k_always);

  for (auto _ : state) {
    benchmark::DoNotOptimize(pmx.cross(p1, p2, c1, c2));
    benchmark::DoNotOptimize(c1);
    benchmark::DoNotOptimize(c2);
  }
}

static void BM_Ox(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  const auto p1 = bench::operators::make_permutation(size);
  auto p2 = bench::operators::make_permutation(size);
  std::reverse(p2.begin(), p2.end());
  std::vector<size_t> c1(size);
  std::vector<size_t> c2(size);
  metah::permut::Ox<> ox(bench::operators::k_always);

  for (auto _ : state) {
    benchmark::DoNotOptimize(ox.cross(p1, p2, c1, c2));
    benchmark::DoNotOptimize(c1);
    benchmark::DoNotOptimize(c2);
  }
}

BENCHMARK(BM_Pmx)->RangeMultiplier(4)->Range(16, 65536);
BENCHMARK(BM_Ox)->RangeMultiplier(4)->Range(16, 65536);
