#include <benchmark/benchmark.h>

#include <metah/operators/binary/flip_mut.hpp>

#include "common.hpp"

static void BM_FlipMut(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  std::vector<uint8_t> repr(size);
  metah::binary::FlipMut<> flip(bench::operators::flip_chance(size));

  for (auto _ : state) {
    benchmark::DoNotOptimize(flip.mutate(repr));
    benchmark::DoNotOptimize(repr);
  }
}

BENCHMARK(BM_FlipMut)->RangeMultiplier(4)->Range(16, 65536);
