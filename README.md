# metah

C++20 metaheuristics library: genetic algorithms, simulated annealing, tabu
search, and fully customizable operators.

Algorithms are **representation- and score-agnostic**. Any type can be used as a
solution encoding or fitness value as long as you supply matching operators
(and a problem that evaluates them). Binary and permutation encodings ship with
ready-made operators under `metah/operators/binary` and
`metah/operators/permut`; everything else is open for your own implementations.

## Features

- **Algorithms:** genetic algorithm, simulated annealing, tabu search — work
  with any representation and score type
- **Operators:** mutation, crossover, neighbourhood, perturbation, selection —
  all pluggable; implement the abstract interfaces for full customization
- **Built-in encodings:** pre-implemented binary and permutation operators
- **Observers:** solution / population logging hooks
- **RNG:** `ThreadLocalRandom` / `GlobalRandom` (`boost::random::xoshiro256pp`)
- **Parallelism:** OpenMP in GA and tabu search

See `examples/` (e.g. TSP) for end-to-end usage.

## Quick start

Requires [vcpkg](https://vcpkg.io) and `VCPKG_ROOT` set.

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
```

Workflow preset (configure + build + test):

```bash
cmake --workflow --preset release
```

### Build options

| Option | Default (top-level) | Meaning |
|--------|---------------------|---------|
| `METAH_BUILD_TESTS` | `ON` | Build doctest suite + benchmarks |
| `METAH_BUILD_EXAMPLES` | `ON` | Build example programs |

When consumed as a subdirectory or installed package, both default to `OFF`.

### As a dependency

```cmake
find_package(metah CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE metah::metah)
```

`metah::metah` is an INTERFACE target. Linking it propagates C++20, includes
(`#include <metah/...>`), OpenMP, Boost.Random, {fmt}, and Abseil.

## Compatibility

### Requirements

| Component | Requirement |
|-----------|-------------|
| Language | C++20 |
| CMake | ≥ 3.25 |
| Package manager | vcpkg (manifest mode) |
| OpenMP | Required (CXX) |
| Boost.Random | ≥ 1.91 (`xoshiro256pp`) |
| {fmt} | ≥ 12.1 |
| Abseil | ≥ 20260107 (`absl::linked_hash_set`) |

**Optional** (vcpkg feature `tests`, enabled by default in presets):

| Component | Requirement |
|-----------|-------------|
| doctest | ≥ 2.5.2 |
| Google Benchmark | ≥ 1.9.5 |

### Platforms

| Platform | Notes |
|----------|-------|
| Windows | Supported |
| Linux | Supported |
| macOS | Untested; OpenMP often needs extra setup (e.g. libomp) |

### Compilers

Needs C++20 with `std::span`, CTAD for standard distributions, and OpenMP
pragmas used by GA / tabu search.

| Compiler | Expected status |
|----------|-----------------|
| GCC 12+ | Supported |
| Clang 15+ | Supported |
| MSVC / clang-cl (VS 2022+) | Supported with OpenMP enabled |
| Apple Clang | Untested; OpenMP usually needs extra setup |

### Known limitations

- OpenMP is mandatory at configure time even if you never run parallel algorithms.
- Boost.Random ≥ 1.91 is required for `boost/random/xoshiro.hpp`.
- Not validated on 32-bit targets.

## Layout

```
src/metah/           library headers
  algorithms/        GA, SA, tabu search
  operators/         bases + binary/ + permut/
  observers/         logging observers
  util/              RNG and helpers
examples/            sample problems (TSP, knapsack)
test/                doctest suite
bench/               microbenchmarks
```

## Versioning

- Project version: **0.1.0**
- Installed package config uses **SameMajorVersion** compatibility.

## Third-party licenses

Dependencies (Boost.Random, {fmt}, Abseil, OpenMP runtime, and optional
test/bench packages) are covered by their own licenses. See
[THIRD_PARTY.md](THIRD_PARTY.md).

## Roadmap / TODOs

- **Documentation:** public API docs, usage guides, and example write-ups.
- **Fewer dependencies:** trim external deps where practical (e.g. replace
  Boost.Random with a smaller in-tree RNG).
- **More flexible parallelism:** move toward a more portable parallel backend
  so algorithms are easier to use across platforms without being tied to OpenMP.
