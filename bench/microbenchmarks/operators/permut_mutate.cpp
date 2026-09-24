#include <benchmark/benchmark.h>

#include <metah/operators/permut/insert_mut.hpp>
#include <metah/operators/permut/reverse_mut.hpp>
#include <metah/operators/permut/swap_mut.hpp>

#include "common.hpp"

static void BM_SwapMut(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  auto repr = bench::operators::make_permutation(size);
  metah::permut::Swap swap(bench::operators::k_always);

  for (auto _ : state) {
    benchmark::DoNotOptimize(swap.mutate(repr));
    benchmark::DoNotOptimize(repr);
  }
}

static void BM_ReverseMut(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  std::vector<size_t> repr(size);
  metah::permut::Reverse reverse(bench::operators::k_always);

  for (auto _ : state) {
    benchmark::DoNotOptimize(reverse.mutate(repr));
    benchmark::DoNotOptimize(repr);
  }
}

static void BM_InsertMut(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  std::vector<size_t> repr(size);
  metah::permut::Insert insert(bench::operators::k_always);

  for (auto _ : state) {
    benchmark::DoNotOptimize(insert.mutate(repr));
    benchmark::DoNotOptimize(repr);
  }
}

BENCHMARK(BM_SwapMut)->RangeMultiplier(4)->Range(16, 65536);
BENCHMARK(BM_ReverseMut)->RangeMultiplier(4)->Range(16, 65536);
BENCHMARK(BM_InsertMut)->RangeMultiplier(4)->Range(16, 65536);
