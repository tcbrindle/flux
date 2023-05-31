
add_library(catch-main catch_main.cpp)
target_compile_features(catch-main PUBLIC cxx_std_20)
target_compile_definitions(catch-main PUBLIC -DCATCH_CONFIG_NO_POSIX_SIGNALS)

add_executable(test-libflux

    test_concepts.cpp
    test_overflow.cpp
    test_optional.cpp
    test_predicates.cpp
    test_simple_sequence.cpp
    test_apply.cpp

    test_adjacent.cpp
    test_adjacent_map.cpp
    test_all_any_none.cpp
    test_bounds_checked.cpp
    test_cache_last.cpp
    test_cartesian_product.cpp
    test_cartesian_product_with.cpp
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
    test_find.cpp
    test_flatten.cpp
    test_for_each.cpp
    test_fold.cpp
    test_front_back.cpp
    test_generator.cpp
    test_map.cpp
    test_minmax.cpp
    test_output_to.cpp
    test_range_iface.cpp
    test_read_only.cpp
    test_reverse.cpp
    test_scan.cpp
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
target_link_libraries(test-libflux flux catch-main)
target_compile_definitions(test-libflux PUBLIC FLUX_UNWIND_ON_ERROR FLUX_ERROR_ON_OVERFLOW)

if(${FLUX_ENABLE_ASAN})
    target_compile_options(test-libflux PRIVATE -fsanitize=address)
    target_link_options(test-libflux PRIVATE -fsanitize=address)
endif()

if(${FLUX_ENABLE_UBSAN})
    target_compile_options(test-libflux PRIVATE -fsanitize=undefined)
    target_link_options(test-libflux PRIVATE -fsanitize=undefined)
endif()

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    target_precompile_headers(test-libflux PRIVATE "catch.hpp")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(test-libflux PRIVATE -Wall -Wextra -pedantic
        -fno-omit-frame-pointer
        -ftemplate-backtrace-limit=0
        -fconcepts-diagnostics-depth=2)
endif()

if(MSVC)
    target_compile_options(test-libflux PRIVATE /W4
        /wd4459 # local variable name hides global variable
    )
endif()

include(Catch)
catch_discover_tests(test-libflux)
