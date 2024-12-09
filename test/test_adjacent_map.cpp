
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>

#include "test_utils.hpp"

namespace {

constexpr bool test_pairwise_map() {
    constexpr auto tuple_sum = [](auto... args) {
        return (args + ...);
    };

    // Basic pairwise_map
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::ref(arr).pairwise_map(tuple_sum);

        using S = decltype(seq);

        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_iterable<S>);

        STATIC_CHECK(seq.size() ==  4);

        STATIC_CHECK(check_equal(seq, {3, 5, 7, 9}));
    }

    // Can const-iterate
    {
        std::array arr{1, 2, 3, 4, 5};

        auto const seq = flux::pairwise_map(arr, tuple_sum);

        using S = decltype(seq);

        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_iterable<S>);

        STATIC_CHECK(flux::size(seq) ==  4);

        STATIC_CHECK(check_equal(seq, {3, 5, 7, 9}));
    }

    // Short sequence -> empty sequence
    {
        auto seq = flux::single(3).pairwise_map(tuple_sum);

        static_assert(seq.is_empty());
        static_assert(seq.size() == 0);
        static_assert(seq.is_last(seq.first()));
    }

    // Empty sequence -> empty sequence
    {
        auto seq = flux::pairwise_map(flux::empty<int>, tuple_sum);

        static_assert(seq.is_empty());
        static_assert(seq.size() == 0);
        static_assert(seq.is_last(seq.first()));
    }

    // pairwise_map with 2-element sequence has one element
    {
        auto seq = flux::pairwise_map(std::array{1, 2}, tuple_sum);

        STATIC_CHECK(seq.size() == 1);
        STATIC_CHECK(seq.front().value() == 3);
    }

    // Reverse iteration works as expected
    {
        std::array arr{1, 2, 3, 4, 5};
        auto seq = flux::ref(arr).pairwise_map(tuple_sum).reverse();

        STATIC_CHECK(seq.size() == 4);
        STATIC_CHECK(check_equal(seq, {9, 7, 5, 3}));
    }

    return true;
}
static_assert(test_pairwise_map());

constexpr bool test_adjacent_map()
{
    constexpr auto tuple_sum = [](auto... args) {
        return (args + ...);
    };

    // Basic pairwise_map
    {
        std::array arr{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        auto seq = flux::ref(arr).adjacent_map<4>(tuple_sum);

        using S = decltype(seq);

        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_iterable<S>);

        STATIC_CHECK(seq.size() ==  7);

        STATIC_CHECK(check_equal(seq, {10, 14, 18, 22, 26, 30, 34}));
    }

    // Can const-iterate
    {
        std::array arr{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        auto const seq = flux::adjacent_map<4>(arr, tuple_sum);

        using S = decltype(seq);

        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_iterable<S>);

        STATIC_CHECK(flux::size(seq) ==  7);

        STATIC_CHECK(check_equal(seq, {10, 14, 18, 22, 26, 30, 34}));
    }

    // Short sequence -> empty sequence
    {
        auto seq = flux::single(3).adjacent_map<10>(tuple_sum);

        static_assert(seq.is_empty());
        static_assert(seq.size() == 0);
        static_assert(seq.is_last(seq.first()));
    }

    // Empty sequence -> empty sequence
    {
        auto seq = flux::adjacent_map<5>(flux::empty<int>, tuple_sum);

        static_assert(seq.is_empty());
        static_assert(seq.size() == 0);
        static_assert(seq.is_last(seq.first()));
    }

    // adjacent_map<N> with N-element sequence has one element
    {
        auto seq = flux::adjacent_map<5>(std::array{1, 2, 3, 4, 5}, tuple_sum);

        STATIC_CHECK(seq.size() == 1);
        STATIC_CHECK(seq.front().value() == 15);
    }

    // Reverse iteration works as expected
    {
        std::array arr{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        auto seq = flux::ref(arr).adjacent_map<4>(tuple_sum).reverse();

        STATIC_CHECK(seq.size() == 7);
        STATIC_CHECK(check_equal(seq, {34, 30, 26, 22, 18, 14, 10}));
    }

    return true;
}
static_assert(test_adjacent_map());

}

TEST_CASE("pairwise_map")
{
    bool res = test_pairwise_map();
    REQUIRE(res);

    res = test_adjacent_map();
    REQUIRE(res);
}