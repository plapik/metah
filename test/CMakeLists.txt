find_package(doctest CONFIG REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${doctest_DIR}")

add_executable(meta_tests
        test_main.cpp
        test_rng.hpp
        operators/permut/swap_mut.cpp
        operators/permut/reverse_mut.cpp
        operators/permut/ox.cpp
        operators/permut/pmx.cpp
        operators/permut/insert_mut.cpp
        operators/permut/reverse_nbr.cpp
        operators/permut/swap_nbr.cpp
        operators/permut/reverse_perturbation.cpp
        operators/permut/swap_perturbation.cpp
        operators/tournament.cpp
        operators/binary/single_point_cross.cpp
        operators/binary/bitset_single_point_cross.cpp
        operators/binary/flip_mut.cpp
        operators/binary/flip_nbr.cpp
        operators/binary/flip_perturbation.cpp
        util/dynamic_bitset.cpp
)

target_include_directories(meta_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(meta_tests PRIVATE
        metah::metah
        doctest::doctest
)

include(doctest)
doctest_discover_tests(meta_tests)