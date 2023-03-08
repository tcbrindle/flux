
cmake_minimum_required(VERSION 3.3)
cmake_policy(SET CMP0092 NEW)

project(libflux CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_library(flux INTERFACE)
target_sources(flux INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/flux.hpp
)

if (MSVC)
    target_compile_features(flux INTERFACE cxx_std_23)
    target_compile_options(flux INTERFACE /permissive-)
else()
    target_compile_features(flux INTERFACE cxx_std_20)
endif()

target_include_directories(flux INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)


if (MSVC)
    target_compile_options(flux INTERFACE /permissive-)
endif()

set(CMAKE_CXX_EXTENSIONS Off)

option(FLUX_BUILD_DOCS "Build Flux documentation (requires Sphinx)" Off)
option(FLUX_BUILD_EXAMPLES "Build Flux examples" On)
option(FLUX_BUILD_TESTS "Build Flux tests" On)
option(FLUX_BUILD_BENCHMARKS "Build Flux benchmarks" Off)
option(FLUX_BUILD_TOOLS "Build single-header generator tool" Off)
option(FLUX_ENABLE_ASAN "Enable Address Sanitizer for tests" Off)
option(FLUX_ENABLE_UBSAN "Enable Undefined Behaviour Sanitizer for tests" Off)

if (${FLUX_BUILD_DOCS})
    add_subdirectory(docs)
endif()

if (${FLUX_BUILD_EXAMPLES})
    enable_testing()
    add_subdirectory(example)
endif()

if (${FLUX_BUILD_BENCHMARKS})
    add_subdirectory(benchmark)
endif()

if (${FLUX_BUILD_TESTS})
    enable_testing()
    add_subdirectory(test)
endif()

if (${FLUX_BUILD_TOOLS})
    add_subdirectory(tools)
endif()