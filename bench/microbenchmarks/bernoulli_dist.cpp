#include <random>
#include <benchmark/benchmark.h>
#include <boost/random/xoshiro.hpp>

static void BM_Bernoulli(benchmark::State& state) {
  const double chance = state.range(0) / 100.0;

  boost::random::xoshiro256pp rng(12345);
  std::uint64_t count = 0;

  for (auto _ : state) {
    benchmark::DoNotOptimize(count);
    std::bernoulli_distribution dist(chance);
    if (dist(rng)) ++count;
  }

  benchmark::DoNotOptimize(count);
}

static void BM_UniformDoubleCompare(benchmark::State& state) {
  const double chance = state.range(0) / 100.0;

  boost::random::xoshiro256pp rng(12345);
  std::uint64_t count = 0;

  for (auto _ : state) {
    benchmark::DoNotOptimize(count);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    if (dist(rng) < chance) ++count;
  }

  benchmark::DoNotOptimize(count);
}

static void BM_GenerateCanonical(benchmark::State& state) {
  const double chance = state.range(0) / 100.0;

  boost::random::xoshiro256pp rng(12345);
  std::uint64_t count = 0;

  for (auto _ : state) {
    benchmark::DoNotOptimize(count);

    if (std::generate_canonical<double, 53>(rng) < chance) ++count;
  }

  benchmark::DoNotOptimize(count);
}

BENCHMARK(BM_Bernoulli)->Arg(1)->Arg(10)->Arg(50)->Arg(90);

BENCHMARK(BM_UniformDoubleCompare)->Arg(1)->Arg(10)->Arg(50)->Arg(90);

BENCHMARK(BM_GenerateCanonical)->Arg(1)->Arg(10)->Arg(50)->Arg(90);
