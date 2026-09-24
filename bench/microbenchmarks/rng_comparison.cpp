#include <cstdlib>
#include <random>
#include <benchmark/benchmark.h>
#include <boost/random/xoshiro.hpp>
#include <metah/util/rng.hpp>


static void BM_Xoshiro256pp(benchmark::State& state) {
  boost::random::xoshiro256pp rng(42);
  for (auto _ : state) {
    auto r = rng();
    benchmark::DoNotOptimize(r);
  }
}

BENCHMARK(BM_Xoshiro256pp);

static void BM_Lcg64(benchmark::State& state) {
  std::linear_congruential_engine<uint64_t, 6364136223846793005ULL,
                                  1442695040888963407ULL, 0>
      rng(42);
  for (auto _ : state) {
    auto r = rng();
    benchmark::DoNotOptimize(r);
  }
}

BENCHMARK(BM_Lcg64);

static void BM_Mt19937_64(benchmark::State& state) {
  std::mt19937_64 rng(42);
  for (auto _ : state) {
    auto r = rng();
    benchmark::DoNotOptimize(r);
  }
}

BENCHMARK(BM_Mt19937_64);

static void BM_Ranlux48(benchmark::State& state) {
  std::ranlux48 rng(42);
  for (auto _ : state) {
    auto r = rng();
    benchmark::DoNotOptimize(r);
  }
}

BENCHMARK(BM_Ranlux48);

static void BM_ThreadLocalRandom(benchmark::State& state) {
  for (auto _ : state) {
    auto r = metah::ThreadLocalRandom::randomEngine()();
    benchmark::DoNotOptimize(r);
  }
}

BENCHMARK(BM_ThreadLocalRandom);

static void BM_GlobalRandom(benchmark::State& state) {
  for (auto _ : state) {
    auto r = metah::GlobalRandom::randomEngine()();
    benchmark::DoNotOptimize(r);
  }
}

BENCHMARK(BM_GlobalRandom);
