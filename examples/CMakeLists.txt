find_package(fmt CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

add_executable(tsp_example
        tsp_example.cpp
        tsp_evaluator.hpp
)

target_link_libraries(tsp_example PRIVATE
        metah::metah
        fmt::fmt
)
