
include(FetchContent)

FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG ae7a13539fb71f270b87eb2e874fbac80bc8dda2 # v2.4.11
    FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(doctest)

add_executable(test-flux

    num/test_concepts.cpp
    num/test_casts.cpp
    num/test_unchecked_ops.cpp
    num/test_wrapping_ops.cpp
    num/test_overflowing_ops.cpp
    num/test_checked_ops.cpp
    num/test_default_ops.cpp

    test_concepts.cpp
    test_optional.cpp
    test_predicates.cpp
    test_apply.cpp

    test_adjacent.cpp
    test_adjacent_filter.cpp
    test_adjacent_map.cpp
    test_all_any_none.cpp
    test_bounds_checked.cpp
    test_cache_last.cpp
    test_cartesian_power.cpp
    test_cartesian_power_map.cpp
    test_cartesian_product.cpp
    test_cartesian_product_map.cpp
    test_chain.cpp
    test_chunk.cpp
    test_chunk_by.cpp
    test_contains.cpp
    test_compare.cpp
    test_count.cpp
    test_count_if.cpp
    test_cursors.cpp
    test_cycle.cpp
    test_drop.cpp
    test_drop_while.cpp
    test_ends_with.cpp
    test_equal.cpp
    test_fill.cpp
    test_filter.cpp
    test_filter_map.cpp
    test_find.cpp
    test_find_if.cpp
    test_find_if_not.cpp
    test_find_min_max.cpp
    test_flatten.cpp
    test_flatten_with.cpp
    test_for_each.cpp
    test_fold.cpp
    test_front_back.cpp
    test_generator.cpp
    test_map.cpp
    test_mask.cpp
    test_minmax.cpp
    test_output_to.cpp
    test_permutations.cpp
    test_permutations_sized.cpp
    test_range_iface.cpp
    test_read_only.cpp
    test_reverse.cpp
    test_scan.cpp
    test_set_adaptors.cpp
    test_slide.cpp
    test_split.cpp
    test_sort.cpp
    test_starts_with.cpp
    test_stride.cpp
    test_take.cpp
    test_take_while.cpp
    test_to.cpp
    test_unchecked.cpp
    test_write_to.cpp
    test_zip.cpp
    test_zip_map.cpp
    test_zip_algorithms.cpp

    test_array_ptr.cpp
    test_bitset.cpp
    test_empty.cpp
    test_from_range.cpp
    test_getlines.cpp
    test_iota.cpp
    test_istream.cpp
    test_istreambuf.cpp
    test_repeat.cpp
    test_single.cpp
    test_unfold.cpp
)

target_link_libraries(test-flux flux-internal doctest::doctest_with_main)
target_compile_definitions(test-flux PUBLIC
    FLUX_UNWIND_ON_ERROR
    FLUX_ERROR_ON_OVERFLOW
    FLUX_ERROR_ON_DIVIDE_BY_ZERO
    FLUX_DISABLE_STATIC_BOUNDS_CHECKING
    DOCTEST_CONFIG_VOID_CAST_EXPRESSIONS
)
target_precompile_headers(test-flux PRIVATE <doctest/doctest.h>)

if(FLUX_ENABLE_ASAN)
    target_compile_options(test-flux PRIVATE -fsanitize=address)
    target_link_options(test-flux PRIVATE -fsanitize=address)
endif()

if(FLUX_ENABLE_UBSAN)
    target_compile_options(test-flux PRIVATE -fsanitize=undefined)
    target_link_options(test-flux PRIVATE -fsanitize=undefined)
endif()

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.28.0")
    if (FLUX_BUILD_TESTS_USING_MODULE)
        target_sources(test-flux PUBLIC
            FILE_SET CXX_MODULES
            BASE_DIRS ${PROJECT_SOURCE_DIR}/module
            FILES ${PROJECT_SOURCE_DIR}/module/flux.cpp
        )

        target_compile_definitions(test-flux PRIVATE -DUSE_MODULES)
        set_target_properties(test-flux PROPERTIES CXX_SCAN_FOR_MODULES On)
    endif()
endif()

if(FLUX_BUILD_MODULE)
    add_executable(test-module-import test_module_import.cpp)
    target_link_libraries(test-module-import PUBLIC flux-mod)
    set_target_properties(test-module-import PROPERTIES
                          CXX_EXTENSIONS Off
                          CXX_SCAN_FOR_MODULES On)
endif()

list(APPEND CMAKE_MODULE_PATH ${doctest_SOURCE_DIR}/scripts/cmake)
include(doctest)
doctest_discover_tests(test-flux)
