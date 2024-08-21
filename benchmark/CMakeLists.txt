
include(FetchContent)

FetchContent_Declare(
    nanobench
    GIT_REPOSITORY https://github.com/martinus/nanobench.git
    GIT_TAG v4.3.11
)

FetchContent_MakeAvailable(nanobench)

add_executable(benchmark-internal-iteration internal_iteration_benchmark.cpp)
target_link_libraries(benchmark-internal-iteration PUBLIC nanobench::nanobench flux)

add_executable(benchmark-sort sort_benchmark.cpp)
target_link_libraries(benchmark-sort PUBLIC nanobench::nanobench flux)

add_executable(benchmark-multidimensional-memset multidimensional_memset_benchmark.cpp multidimensional_memset_benchmark_kernels.cpp)
target_link_libraries(benchmark-multidimensional-memset PUBLIC nanobench::nanobench flux)
