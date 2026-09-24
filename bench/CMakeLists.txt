find_package(benchmark CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(metah_benchmark
        microbenchmarks/main.cpp
        microbenchmarks/rng_comparison.cpp
        microbenchmarks/bernoulli_dist.cpp
        microbenchmarks/operators/selection.cpp
        microbenchmarks/operators/binary_mutate.cpp
        microbenchmarks/operators/binary_crossover.cpp
        microbenchmarks/operators/binary_neighbour.cpp
        microbenchmarks/operators/binary_perturbation.cpp
        microbenchmarks/operators/permut_mutate.cpp
        microbenchmarks/operators/permut_crossover.cpp
        microbenchmarks/operators/permut_neighbour.cpp
        microbenchmarks/operators/permut_perturbation.cpp
)

target_link_libraries(metah_benchmark
        PRIVATE metah::metah
        PRIVATE benchmark::benchmark
)

target_include_directories(metah_benchmark PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/microbenchmarks/operators
)

if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(metah_benchmark PRIVATE /openmp:llvm)
endif()