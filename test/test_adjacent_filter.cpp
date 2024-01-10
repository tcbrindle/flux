
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>

#include "test_utils.hpp"

namespace {

constexpr bool test_adjacent_filter()
{
    // Basic adjacent_filter
    {
        std::array arr{1, 1, 1, 2, 2, 3, 4, 4, 4, 5};

        auto filtered = flux::adjacent_filter(arr, std::not_equal_to{});

        using F = decltype(filtered);
        static_assert(flux::sequence<F>);
        static_assert(flux::multipass_sequence<F>);
        static_assert(flux::bidirectional_sequence<F>);
        static_assert(not flux::random_access_sequence<F>);
        static_assert(flux::bounded_sequence<F>);
        static_assert(not flux::sized_sequence<F>);

        static_assert(std::same_as<flux::element_t<F>, int&>);

        STATIC_CHECK(filtered.count() == 5);
        STATIC_CHECK(check_equal(filtered, {1, 2, 3, 4, 5}));
        STATIC_CHECK(filtered.is_last(filtered.last()));
    }

    // adjacent_filter is const-iterable when the base is
    {
        std::array arr{1, 1, 1, 2, 2, 3, 4, 4, 4, 5};

        auto const filtered = flux::adjacent_filter(arr, std::not_equal_to{});

        using F = decltype(filtered);
        static_assert(flux::sequence<F>);
        static_assert(flux::multipass_sequence<F>);
        static_assert(flux::bidirectional_sequence<F>);
        static_assert(not flux::random_access_sequence<F>);
        static_assert(flux::bounded_sequence<F>);
        static_assert(not flux::sized_sequence<F>);

        static_assert(std::same_as<flux::element_t<F>, int const&>);

        STATIC_CHECK(flux::count(filtered) == 5);
        STATIC_CHECK(check_equal(filtered, {1, 2, 3, 4, 5}));
        STATIC_CHECK(flux::is_last(filtered, flux::last(filtered)));
    }

    // adj_filt of an empty sequence is empty
    {
        auto f = flux::adjacent_filter(flux::empty<int>, std::not_equal_to{});

        STATIC_CHECK(f.is_empty());
    }

    // adj_filt with a sequence of size 1 is just that element
    {
        auto f = flux::single(99).adjacent_filter(std::not_equal_to{});

        STATIC_CHECK(f.count() == 1);
        STATIC_CHECK(f.front().value() == 99);
    }

    // adj_filt with a repeated sequence contains one element
    {
        auto f = flux::adjacent_filter(flux::repeat(99, 1000), std::not_equal_to{});

        STATIC_CHECK(f.count() == 1);
        STATIC_CHECK(f.front().value() == 99);
    }

    // adj_filt picks the first of a run of identical elements
    {
        struct Pair { int i, j; bool operator==(const Pair&) const = default; };

        Pair arr[] = { {1, 1}, {1, 2}, {1, 3}, {2, 4}, {2, 5}};

        auto filtered = flux::ref(arr).adjacent_filter(
            flux::proj{std::not_equal_to{}, &Pair::i});

        STATIC_CHECK(filtered.count() == 2);
        STATIC_CHECK(check_equal(filtered, {Pair{1, 1}, Pair{2, 4}}));
    }

    // ...and again, slightly differently
    {
        int arr[] = {1, 1, 1, 2, 2, 2, 3, 3, 3};

        auto filtered = flux::ref(arr).adjacent_filter(std::not_equal_to{});

        STATIC_CHECK(filtered.count() == 3);

        auto cur = filtered.first();

        STATIC_CHECK(&filtered[cur] == arr);
        STATIC_CHECK(&filtered[filtered.inc(cur)] == arr + 3);
        STATIC_CHECK(&filtered[filtered.inc(cur)] == arr + 6);
    }

    // adj_filter of a bidir sequence is bidir
    {
        int arr[] = {1, 1, 1, 3, 3, 3, 2, 2, 2};

        auto seq = flux::ref(arr).adjacent_filter(std::not_equal_to{}).reverse();

        STATIC_CHECK(check_equal(seq, {2, 3, 1}));

        auto cur = seq.first();
        STATIC_CHECK(&seq[cur] == arr + 6);
        STATIC_CHECK(&seq[seq.inc(cur)] == arr + 3);
        STATIC_CHECK(&seq[seq.inc(cur)] == arr);
    }

    return true;
}
static_assert(test_adjacent_filter());

constexpr bool test_dedup()
{
    // Basic dedup
    {
        std::array arr{1, 1, 1, 2, 2, 3, 4, 4, 4, 5};

        auto filtered = flux::dedup(arr);

        using F = decltype(filtered);
        static_assert(flux::sequence<F>);
        static_assert(flux::multipass_sequence<F>);
        static_assert(flux::bidirectional_sequence<F>);
        static_assert(not flux::random_access_sequence<F>);
        static_assert(flux::bounded_sequence<F>);
        static_assert(not flux::sized_sequence<F>);

        static_assert(std::same_as<flux::element_t<F>, int&>);

        STATIC_CHECK(filtered.count() == 5);
        STATIC_CHECK(check_equal(filtered, {1, 2, 3, 4, 5}));
        STATIC_CHECK(filtered.is_last(filtered.last()));
    }

    // dedup is const-iterable when the base is
    {
        std::array arr{1, 1, 1, 2, 2, 3, 4, 4, 4, 5};

        auto const filtered = flux::dedup(arr);

        using F = decltype(filtered);
        static_assert(flux::sequence<F>);
        static_assert(flux::multipass_sequence<F>);
        static_assert(flux::bidirectional_sequence<F>);
        static_assert(not flux::random_access_sequence<F>);
        static_assert(flux::bounded_sequence<F>);
        static_assert(not flux::sized_sequence<F>);

        static_assert(std::same_as<flux::element_t<F>, int const&>);

        STATIC_CHECK(flux::count(filtered) == 5);
        STATIC_CHECK(check_equal(filtered, {1, 2, 3, 4, 5}));
        STATIC_CHECK(flux::is_last(filtered, flux::last(filtered)));
    }

    // dedup of an empty sequence is empty
    {
        auto f = flux::dedup(flux::empty<int>);

        STATIC_CHECK(f.is_empty());
    }

    // dedup with a sequence of size 1 is just that element
    {
        auto f = flux::single(99).dedup();

        STATIC_CHECK(f.count() == 1);
        STATIC_CHECK(f.front().value() == 99);
    }

    // dedup with a repeated sequence contains one element
    {
        auto f = flux::repeat(99, 1000).dedup();

        STATIC_CHECK(f.count() == 1);
        STATIC_CHECK(f.front().value() == 99);
    }

    // dedup picks the first of a run of equal elements
    {
        int arr[] = {1, 1, 1, 2, 2, 2, 3, 3, 3};

        auto filtered = flux::ref(arr).dedup();

        STATIC_CHECK(filtered.count() == 3);

        auto cur = filtered.first();

        STATIC_CHECK(&filtered[cur] == arr);
        STATIC_CHECK(&filtered[filtered.inc(cur)] == arr + 3);
        STATIC_CHECK(&filtered[filtered.inc(cur)] == arr + 6);
    }

    // dedup of a bidir sequence is bidir
    {
        int arr[] = {1, 1, 1, 3, 3, 3, 2, 2, 2};

        auto seq = flux::ref(arr).dedup().reverse();

        STATIC_CHECK(check_equal(seq, {2, 3, 1}));

        auto cur = seq.first();
        STATIC_CHECK(&seq[cur] == arr + 6);
        STATIC_CHECK(&seq[seq.inc(cur)] == arr + 3);
        STATIC_CHECK(&seq[seq.inc(cur)] == arr);
    }

    return true;
}
static_assert(test_dedup());

}

TEST_CASE("adjacent_filter")
{
    bool res = test_adjacent_filter();
    REQUIRE(res);
}