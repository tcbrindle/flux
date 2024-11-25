
// Copyright (c) 2024 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <optional>
#include <utility>

#include "test_utils.hpp"

namespace {

constexpr auto is_even_opt = [](int i) { return i % 2 == 0 ? std::optional{i} : std::nullopt; };

struct Pair {
    int i;
    bool ok;

    [[nodiscard]] constexpr auto map_if_ok() const { return ok ? std::optional{*this} : std::nullopt; }

    constexpr bool operator==(const Pair&) const = default;
};

using filter_fn = decltype(flux::filter_map);

// int is not a sequence
static_assert(not std::invocable<filter_fn, int, decltype(is_even_opt)>);
// int is not a function
static_assert(not std::invocable<filter_fn, int(&)[10], int>);
// "func" does not return optional_like
static_assert(not std::invocable<filter_fn, int(&)[10], decltype([](int) {})>);
// Incompatible predicate
static_assert(not std::invocable<filter_fn, int(&)[10], decltype([](int*) { return true; })>);

constexpr bool test_filter()
{
    // Basic filtering
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::filter_map(flux::ref(arr), is_even_opt);
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

    // Basic filtering of non-sequence iterables
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto rng = arr | std::views::transform(std::identity{});
        auto filtered = flux::filter_map(std::move(rng), is_even_opt);

        using F = decltype(filtered);
        static_assert(flux::iterable<F>);
        static_assert(flux::const_iterable<F>);
        static_assert(not flux::sequence<F>);

        STATIC_CHECK(check_equal(filtered, {0, 2, 4, 6, 8}));
    }

    // A predicate that always returns true returns what it was given
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::filter_map(flux::ref(arr), [](auto&& i) { return std::optional{i}; });

        STATIC_CHECK(check_equal(arr, filtered));
    }

    // A predicate that always returns false returns an empty sequence
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::filter_map(flux::ref(arr), [](auto&&) -> std::optional<int> { return std::nullopt; });

        STATIC_CHECK(filtered.is_empty());
    }

    // We can use any optional_like such as a pointer
    {
        int i = 1, j = 3;
        std::array<int *, 4> arr{&i, nullptr, &j, nullptr};

        auto filtered = flux::filter_map(arr, [](auto ptr) { return ptr; });

        STATIC_CHECK(check_equal(filtered, {1, 3}));
    }

    // ... Better expressed as filter_deref
    {
        int i = 1, j = 3;
        std::array<int *, 4> arr{&i, nullptr, &j, nullptr};

        auto filtered = flux::filter_deref(arr);

        STATIC_CHECK(check_equal(filtered, {1, 3}));
    }

    // We can use a PMF to filter_map
    {
        std::array<Pair, 4> pairs = {
            Pair{1, true},
            {2, false},
            {3, true},
            {4, false}
        };

        auto f = flux::filter_map(pairs, &Pair::map_if_ok);

        STATIC_CHECK(check_equal(f, {Pair{1, true}, Pair{3, true}}));
    }

    // Reversed sequences can be filtered
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::ref(arr).reverse().filter_map(is_even_opt);

        STATIC_CHECK(check_equal(filtered, {8, 6, 4, 2, 0}));
    }

    // ... and filtered sequences can be reversed
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto filtered = flux::filter_map(flux::ref(arr), is_even_opt).reverse();

        STATIC_CHECK(check_equal(filtered, {8, 6, 4, 2, 0}));
    }

    return true;
}
static_assert(test_filter());

}

TEST_CASE("filter_map")
{
    bool result = test_filter();
    REQUIRE(result);
}
