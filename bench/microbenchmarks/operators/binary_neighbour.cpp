#include <benchmark/benchmark.h>

#include <metah/operators/binary/flip_nbr.hpp>

#include "common.hpp"

static void BM_SingleFlipNeighbour(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  const auto original = bench::operators::make_binary_vector(size);
  std::vector<uint8_t> target(size);
  metah::binary::SingleFlipNeighbour<> neighbour(1);

  for (auto _ : state) {
    benchmark::DoNotOptimize(neighbour.generate(original, target, 0));
    benchmark::DoNotOptimize(target);
  }
}

static void BM_FlipNeighbour(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  const auto original = bench::operators::make_binary_vector(size);
  std::vector<uint8_t> target(size);
  metah::binary::FlipNeighbour<> neighbour(1,
                                           bench::operators::flip_chance(size));

  for (auto _ : state) {
    benchmark::DoNotOptimize(neighbour.generate(original, target, 0));
    benchmark::DoNotOptimize(target);
  }
}

BENCHMARK(BM_SingleFlipNeighbour)->RangeMultiplier(4)->Range(16, 65536);
BENCHMARK(BM_FlipNeighbour)->RangeMultiplier(4)->Range(16, 65536);
