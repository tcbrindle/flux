
include(FetchContent)

FetchContent_Declare(
    nanobench
    GIT_REPOSITORY https://github.com/martinus/nanobench.git
    GIT_TAG v4.3.11
)

FetchContent_MakeAvailable(nanobench)

add_executable(internal-iteration-benchmark internal_iteration_benchmark.cpp)
target_link_libraries(internal-iteration-benchmark PUBLIC nanobench::nanobench flux)

add_executable(sort-benchmark sort_benchmark.cpp)
target_link_libraries(sort-benchmark PUBLIC nanobench::nanobench flux)

add_executable(multidimensional-memset-benchmark multidimensional_memset_benchmark.cpp multidimensional_memset_benchmark_kernels.cpp)
target_link_libraries(multidimensional-memset-benchmark PUBLIC nanobench::nanobench flux)
