#include <benchmark/benchmark.h>

#include <metah/operators/permut/reverse_nbr.hpp>
#include <metah/operators/permut/swap_nbr.hpp>

#include "common.hpp"

static void BM_SwapNeighbour(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  const auto original = bench::operators::make_permutation(size);
  std::vector<size_t> target(size);
  metah::permut::RngSwapNeighbour<> neighbour(1);

  for (auto _ : state) {
    benchmark::DoNotOptimize(neighbour.generate(original, target, 0));
    benchmark::DoNotOptimize(target);
  }
}

static void BM_ReverseNeighbour(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  const auto original = bench::operators::make_permutation(size);
  std::vector<size_t> target(size);
  metah::permut::ReverseNeighbour<> neighbour(1);

  for (auto _ : state) {
    benchmark::DoNotOptimize(neighbour.generate(original, target, 0));
    benchmark::DoNotOptimize(target);
  }
}

BENCHMARK(BM_SwapNeighbour)->RangeMultiplier(4)->Range(16, 65536);
BENCHMARK(BM_ReverseNeighbour)->RangeMultiplier(4)->Range(16, 65536);
