
#include "catch.hpp"

#include "test_utils.hpp"

#include <flux.hpp>

#include <array>
#include <iostream>

namespace {

constexpr auto is_even = [](int i) { return i % 2 == 0; };

struct Pair {
    int a;
    bool ok;

    constexpr int get() const { return a; }
    constexpr bool is_okay() const { return ok; }

    constexpr bool operator==(const Pair&) const = default;
};

using filter_fn = decltype(flux::filter);

// Okay
static_assert(std::invocable<filter_fn, int(&)[10], decltype(is_even)>);
// int is not a sequence
static_assert(not std::invocable<filter_fn, int, decltype(is_even)>);
// int is not a predicate
static_assert(not std::invocable<filter_fn, int(&)[10], int>);
// "predicate" returns void
static_assert(not std::invocable<filter_fn, int(&)[10], decltype([](int) {})>);
// Incompatible predicate
static_assert(not std::invocable<filter_fn, int(&)[10], decltype([](int*) { return true; })>);


constexpr bool test_filter()
{
    // Basic filtering
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::filter(arr, is_even);
        using F = decltype(filtered);
        static_assert(flux::sequence<F>);
        static_assert(flux::bidirectional_sequence<F>);
        static_assert(flux::bounded_sequence<F>);
        static_assert(not flux::ordered_index<F>);
        static_assert(not flux::sized_sequence<F>);

        static_assert(not flux::sequence<F const>);

        if (!check_equal(filtered, {0, 2, 4, 6, 8})) {
            return false;
        }
    }

    // Filtering single-pass sequences works okay
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = single_pass_only(flux::from(arr)).filter(is_even);
        using F = decltype(filtered);
        static_assert(flux::sequence<F>);
        static_assert(not flux::multipass_sequence<F>);
        static_assert(flux::bounded_sequence<F>);
        static_assert(not flux::sized_sequence<F>);

        if (!check_equal(filtered, {0, 2, 4, 6, 8})) {
            return false;
        }
    }

    // A predicate that always returns true returns what it was given
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::filter(arr, [](auto&&) { return true; });

        if (!check_equal(arr, filtered)) {
            return false;
        }
    }

    // A predicate that always returns false returns an empty sequence
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::filter(arr, [](auto&&) { return false; });

        if (!filtered.is_empty()) {
            return false;
        }
    }

    // We can use a PMD to filter
    {
        std::array<Pair, 4> pairs = {
            Pair{1, true},
            {2, false},
            {3, true},
            {4, false}
        };

        auto f = flux::filter(pairs, &Pair::ok);

        if (!check_equal(f, {Pair{1, true}, Pair{3, true}})) {
            return false;
        }
    }

    // We can use a PMF to filter
    {
        std::array<Pair, 4> pairs = {
            Pair{1, true},
            {2, false},
            {3, true},
            {4, false}
        };

        auto f = flux::filter(pairs, &Pair::is_okay);

        if (!check_equal(f, {Pair{1, true}, Pair{3, true}})) {
            return false;
        }
    }

    // Reversed sequences can be filtered
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::reverse(arr).filter(is_even);

        if (!check_equal(filtered, {8, 6, 4, 2, 0})) {
            return false;
        }
    }

    // ... and filtered sequences can be reversed
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::filter(arr, is_even).reverse();

        if (!check_equal(filtered, {8, 6, 4, 2, 0})) {
            return false;
        }
    }

    return true;
}
static_assert(test_filter());

}

TEST_CASE("filter")
{
    bool result = test_filter();
    REQUIRE(result);
}