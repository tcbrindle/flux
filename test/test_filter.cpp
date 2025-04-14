
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <ranges>
#include <utility>

#include "test_utils.hpp"

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
        auto filtered = flux::filter(flux::ref(arr), is_even);
        using F = decltype(filtered);
        static_assert(flux::sequence<F>);
        static_assert(flux::bidirectional_sequence<F>);
        static_assert(flux::bounded_sequence<F>);
        static_assert(not flux::ordered_cursor<F>);
        static_assert(not flux::sized_iterable<F>);

        static_assert(flux::sequence<F const>);
        static_assert(flux::bidirectional_sequence<F const>);
        static_assert(flux::bounded_sequence<F const>);
        static_assert(not flux::ordered_cursor<F const>);
        static_assert(not flux::sized_iterable<F const>);

        STATIC_CHECK(check_equal(filtered, {0, 2, 4, 6, 8}));
        STATIC_CHECK(check_equal(std::as_const(filtered), {0, 2, 4, 6, 8}));
    }

    // Filtering non-sequence iterables works okay
    {
        auto view = std::array{1, 2, 3, 4, 5} | std::views::filter(flux::pred::true_);

        auto filtered = flux::filter(std::move(view), is_even);
        using F = decltype(filtered);

        static_assert(flux::iterable<F>);
        static_assert(!flux::sequence<F>);

        STATIC_CHECK(check_equal(filtered, {2, 4}));
    }

    // Filtering single-pass sequences works okay
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = single_pass_only(flux::ref(arr)).filter(is_even);
        using F = decltype(filtered);
        static_assert(flux::sequence<F>);
        static_assert(not flux::multipass_sequence<F>);
        static_assert(flux::bounded_sequence<F>);
        static_assert(not flux::sized_iterable<F>);

        if (!check_equal(filtered, {0, 2, 4, 6, 8})) {
            return false;
        }
    }

    // A predicate that always returns true returns what it was given
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::filter(flux::ref(arr), [](auto&&) { return true; });

        if (!check_equal(arr, filtered)) {
            return false;
        }
    }

    // A predicate that always returns false returns an empty sequence
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::filter(flux::ref(arr), [](auto&&) { return false; });

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

        auto f = flux::filter(flux::ref(pairs), &Pair::ok);

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

        auto f = flux::filter(std::move(pairs), &Pair::is_okay);

        if (!check_equal(f, {Pair{1, true}, Pair{3, true}})) {
            return false;
        }
    }

    // Reversed sequences can be filtered
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::ref(arr).reverse().filter(is_even);

        if (!check_equal(filtered, {8, 6, 4, 2, 0})) {
            return false;
        }
    }

    // ... and filtered sequences can be reversed
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::filter(flux::ref(arr), is_even).reverse();

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