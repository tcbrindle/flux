
cmake_minimum_required(VERSION 3.3)
cmake_policy(SET CMP0092 NEW)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

project(libflux CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_library(flux INTERFACE)
file(GLOB_RECURSE FLUX_HPPS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp" )
target_sources(flux INTERFACE $<BUILD_INTERFACE:${FLUX_HPPS}>)

if (MSVC)
    target_compile_features(flux INTERFACE cxx_std_23)
    target_compile_options(flux INTERFACE /permissive-)
else()
    target_compile_features(flux INTERFACE cxx_std_20)
endif()

target_include_directories(
    flux 
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set(CMAKE_CXX_EXTENSIONS Off)

option(FLUX_BUILD_DOCS "Build Flux documentation (requires Sphinx)" Off)
option(FLUX_BUILD_EXAMPLES "Build Flux examples" On)
option(FLUX_BUILD_TESTS "Build Flux tests" On)
option(FLUX_BUILD_BENCHMARKS "Build Flux benchmarks" Off)
option(FLUX_BUILD_TOOLS "Build single-header generator tool" Off)
option(FLUX_BUILD_MODULE "Build C++20 module (experimental)" Off)
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

if (${FLUX_BUILD_MODULE})
    add_subdirectory(module)
endif()

set(PORT_NAME flux)
set(CONFIG_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PORT_NAME}")

# header-only doesn't need architeture differences so clear CMAKE_SIZEOF_VOIDP
# temporarily when creating the version file.
set(ORIGINAL_CMAKE_SIZEOF_VOIDP ${CMAKE_SIZEOF_VOIDP})
set(CMAKE_SIZEOF_VOIDP "")
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/${PORT_NAME}-version.cmake"
    VERSION 0.0.1
    COMPATIBILITY SameMajorVersion
)
set(CMAKE_SIZEOF_VOIDP ${ORIGINAL_CMAKE_SIZEOF_VOIDP})

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/${PORT_NAME}-config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PORT_NAME}-config.cmake"
    INSTALL_DESTINATION "${CONFIG_DESTINATION}"
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# set target installation location properties and associates it with the targets files
install(
    TARGETS ${PORT_NAME}
    EXPORT ${PORT_NAME}-targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

#install the targets files
install(
    EXPORT ${PORT_NAME}-targets
    NAMESPACE ${PORT_NAME}::
    DESTINATION "${CONFIG_DESTINATION}"
)

# install the config and version files
install(
    FILES
        "${PROJECT_BINARY_DIR}/${PORT_NAME}-config.cmake"
        "${PROJECT_BINARY_DIR}/${PORT_NAME}-version.cmake"
    DESTINATION "${CONFIG_DESTINATION}"
)

# install the headers
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/.."
    PATTERN "build2file" EXCLUDE
)