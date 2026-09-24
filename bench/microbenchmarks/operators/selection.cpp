#include <benchmark/benchmark.h>

#include <metah/operators/tournament.hpp>

#include "common.hpp"

static void BM_Tournament(benchmark::State& state) {
  const auto scores =
      bench::operators::make_scores(static_cast<size_t>(state.range(0)));
  metah::Tournament<int64_t, true> tournament(5);

  for (auto _ : state) {
    auto idx = tournament.select(scores);
    benchmark::DoNotOptimize(idx);
  }
}

BENCHMARK(BM_Tournament)->RangeMultiplier(4)->Range(16, 65536);
