# Third-party notices

**metah** depends on the following third-party software. Their licenses apply to
those components as distributed by their upstream projects. This file is a
convenience summary, not a substitute for the full license texts shipped with
each dependency (e.g. via vcpkg or your package manager).

## Runtime / link dependencies

| Component | License | Notes |
|-----------|---------|--------|
| [Boost.Random](https://www.boost.org/libs/random/) | [BSL-1.0](https://www.boost.org/LICENSE_1_0.txt) | `xoshiro256pp` and related |
| [{fmt}](https://github.com/fmtlib/fmt) | [MIT](https://github.com/fmtlib/fmt/blob/master/LICENSE) | Formatting / I/O helpers |
| [Abseil](https://abseil.io/) | [Apache-2.0](https://github.com/abseil/abseil-cpp/blob/master/LICENSE) | e.g. `absl::linked_hash_set` |
| OpenMP runtime | Vendor-specific | Comes with your compiler (libgomp, LLVM OpenMP, MSVC OpenMP, etc.). See that toolchain’s license. |

## Optional (tests, benchmarks)

Pulled in when the vcpkg `tests` feature is enabled:

| Component | License |
|-----------|---------|
| [doctest](https://github.com/doctest/doctest) | [MIT](https://github.com/doctest/doctest/blob/master/LICENSE.txt) |
| [Google Benchmark](https://github.com/google/benchmark) | [Apache-2.0](https://github.com/google/benchmark/blob/main/LICENSE) |
