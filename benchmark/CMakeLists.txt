
add_library(nanobench nanobench.cpp)
target_include_directories(nanobench PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(brevzin-benchmark brevzin_benchmark.cpp)
target_link_libraries(brevzin-benchmark PUBLIC nanobench flux)

add_executable(sort-benchmark sort_benchmark.cpp)
target_link_libraries(sort-benchmark PUBLIC nanobench flux)