#include <benchmark/benchmark.h>

#include <metah/operators/binary/flip_perturbation.hpp>

#include "common.hpp"

static void BM_SingleFlipPerturbation(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  const auto current = bench::operators::make_binary_vector(size);
  std::vector<uint8_t> target(size);
  metah::binary::SingleFlipPerturbation<> perturbation;

  for (auto _ : state) {
    perturbation.perturb(current, target);
    benchmark::DoNotOptimize(target);
  }
}

static void BM_FlipPerturbation(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  const auto current = bench::operators::make_binary_vector(size);
  std::vector<uint8_t> target(size);
  metah::binary::FlipPerturbation<> perturbation(
      bench::operators::flip_chance(size));

  for (auto _ : state) {
    perturbation.perturb(current, target);
    benchmark::DoNotOptimize(target);
  }
}

BENCHMARK(BM_SingleFlipPerturbation)->RangeMultiplier(4)->Range(16, 65536);
BENCHMARK(BM_FlipPerturbation)->RangeMultiplier(4)->Range(16, 65536);
