
include(FetchContent)

FetchContent_Declare(
    nanobench
    GIT_REPOSITORY https://github.com/martinus/nanobench.git
    GIT_TAG b709728dba4df0356d467abfb0ba20781a8031c0 # v4.6.0
)

FetchContent_MakeAvailable(nanobench)

add_executable(flux.benchmark.internal_iteration internal_iteration_benchmark.cpp)
target_link_libraries(flux.benchmark.internal_iteration PUBLIC nanobench::nanobench flux::flux)

add_executable(flux.benchmark.sort sort_benchmark.cpp)
target_link_libraries(flux.benchmark.sort PUBLIC nanobench::nanobench flux::flux)

add_executable(flux.benchmark.multidimensional_memset multidimensional_memset_benchmark.cpp multidimensional_memset_benchmark_kernels.cpp)
target_link_libraries(flux.benchmark.multidimensional_memset PUBLIC nanobench::nanobench flux::flux)
