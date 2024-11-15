
cmake_minimum_required(VERSION 3.23)

project(flux CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_EXTENSIONS Off)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_library(flux INTERFACE)
add_library(flux::flux ALIAS flux)

file(GLOB_RECURSE FLUX_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp" )

target_sources(flux INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES ${FLUX_HEADERS})

target_compile_features(flux INTERFACE $<IF:$<CXX_COMPILER_ID:MSVC>,cxx_std_23,cxx_std_20>)
set_target_properties(flux PROPERTIES CXX_STANDARD_REQUIRED On)

add_library(flux-internal INTERFACE)
target_link_libraries(flux-internal INTERFACE flux)
set_target_properties(flux-internal PROPERTIES CXX_EXTENSIONS Off)

target_compile_options(flux-internal INTERFACE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU,Intel>:
        -Wall -Wextra -Wconversion -pedantic
        -fno-omit-frame-pointer
        -ftemplate-backtrace-limit=0
    >

    $<$<CXX_COMPILER_ID:GNU>: -fconcepts-diagnostics-depth=2>

    $<$<CXX_COMPILER_ID:Clang,AppleClang>: -fconstexpr-backtrace-limit=0>

    $<$<CXX_COMPILER_ID:MSVC>:
        # Various options for closer standard conformance
        /utf-8 /Zc:__cplusplus /Zc:throwingNew /Zc:inline /Zc:externConstexpr
        /Zc:templateScope /Zc:checkGwOdr /Zc:enumTypes
        /W4
        /wd4459 # local variable name hides global variable
        /wd4702 # unreachable code
        /wd4100 # unreferenced formal parameter
    >
)

option(FLUX_BUILD_DOCS "Build Flux documentation (requires Sphinx)" Off)
option(FLUX_BUILD_EXAMPLES "Build Flux examples" ${PROJECT_IS_TOP_LEVEL})
option(FLUX_BUILD_TESTS "Build Flux tests" ${PROJECT_IS_TOP_LEVEL})
option(FLUX_BUILD_BENCHMARKS "Build Flux benchmarks" Off)
option(FLUX_BUILD_TOOLS "Build single-header generator tool" Off)
option(FLUX_BUILD_MODULE "Build C++20 module (experimental)" Off)
option(FLUX_BUILD_TESTS_USING_MODULE "Build tests using modules (experimental)" Off)
option(FLUX_ENABLE_ASAN "Enable Address Sanitizer for tests" Off)
option(FLUX_ENABLE_UBSAN "Enable Undefined Behaviour Sanitizer for tests" Off)

if (FLUX_BUILD_DOCS)
    add_subdirectory(docs)
endif()

if (FLUX_BUILD_EXAMPLES)
    enable_testing()
    add_subdirectory(example)
endif()

if (FLUX_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()

if (FLUX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if (FLUX_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if (FLUX_BUILD_MODULE)
    if(CMAKE_VERSION VERSION_LESS "3.28.0")
        message(FATAL_ERROR "Modules support requires CMake v3.28 or later (currently v${CMAKE_VERSION})")
    else()
        add_subdirectory(module)
    endif()
endif()

set(FLUX_INSTALL_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/flux)

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/flux-version.cmake"
    VERSION -1 # When there is a PROJECT_VERSION, remove this line
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/flux-config.cmake.in"
    "${PROJECT_BINARY_DIR}/flux-config.cmake"
    INSTALL_DESTINATION ${FLUX_INSTALL_CMAKE_DIR}
)

# set target installation location properties and associates it with the targets files
install(
    TARGETS flux
    EXPORT flux-targets
    FILE_SET HEADERS
)

#install the targets files
install(
    EXPORT flux-targets
    NAMESPACE flux::
    DESTINATION ${FLUX_INSTALL_CMAKE_DIR}
)

# install the config and version files
install(
    FILES
        "${PROJECT_BINARY_DIR}/flux-config.cmake"
        "${PROJECT_BINARY_DIR}/flux-version.cmake"
    DESTINATION ${FLUX_INSTALL_CMAKE_DIR}
)
