
add_library(catch-main catch_main.cpp)
target_compile_features(catch-main PUBLIC cxx_std_20)

add_executable(test-libflux

    test_concepts.cpp
    test_simple_sequence.cpp

    test_all_any_none.cpp
    test_bounds_checked.cpp
    test_cache_last.cpp
    test_chain.cpp
    test_contains.cpp
    test_count.cpp
    test_count_if.cpp
    test_drop.cpp
    test_drop_while.cpp
    test_equal.cpp
    test_fill.cpp
    test_filter.cpp
    test_for_each.cpp
    test_find.cpp
    test_generator.cpp
    test_map.cpp
    test_output_to.cpp
    test_reverse.cpp
    test_split.cpp
    test_sort.cpp
    test_take.cpp
    test_take_while.cpp
    test_to.cpp
    test_view.cpp
    test_write_to.cpp
    test_zip.cpp

    test_bitset.cpp
    test_empty.cpp
    test_istream.cpp
    test_istreambuf.cpp
    test_single.cpp
)
target_link_libraries(test-libflux flux catch-main)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    target_precompile_headers(test-libflux PRIVATE "catch.hpp")
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(test-libflux PRIVATE -Wall -Wextra -pedantic
        -ftemplate-backtrace-limit=0
        -fconcepts-diagnostics-depth=2)
endif()

if (MSVC)
    target_compile_options(test-libflux PRIVATE /W4 
                           /wd4459 # local variable name hides global variable
                           )
endif()

include(Catch)
catch_discover_tests(test-libflux)