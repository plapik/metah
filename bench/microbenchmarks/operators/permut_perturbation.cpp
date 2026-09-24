#include <benchmark/benchmark.h>

#include <metah/operators/permut/reverse_perturbation.hpp>
#include <metah/operators/permut/swap_perturbation.hpp>

#include "common.hpp"

static void BM_SwapPerturbation(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  const auto current = bench::operators::make_permutation(size);
  std::vector<size_t> target(size);
  metah::permut::SwapPerturbation<> perturbation;

  for (auto _ : state) {
    perturbation.perturb(current, target);
    benchmark::DoNotOptimize(target);
  }
}

static void BM_ReversePerturbation(benchmark::State& state) {
  const auto size = static_cast<size_t>(state.range(0));
  const auto current = bench::operators::make_permutation(size);
  std::vector<size_t> target(size);
  metah::permut::ReversePerturbation<> perturbation;

  for (auto _ : state) {
    perturbation.perturb(current, target);
    benchmark::DoNotOptimize(target);
  }
}

BENCHMARK(BM_SwapPerturbation)->RangeMultiplier(4)->Range(16, 65536);
BENCHMARK(BM_ReversePerturbation)->RangeMultiplier(4)->Range(16, 65536);
