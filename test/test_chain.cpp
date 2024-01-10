
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

#include "test_utils.hpp"

namespace {

constexpr bool test_chain()
{
    // Basic chaining
    {
        int arr1[] = {0, 1, 2};
        int arr2[] = {3, 4, 5};
        int arr3[] = {6, 7, 8};

        auto seq = flux::chain(flux::mut_ref(arr1), flux::mut_ref(arr2), flux::mut_ref(arr3));

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int&>);
        static_assert(std::same_as<flux::value_t<S>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);

        static_assert(flux::random_access_sequence<S const>);
        static_assert(flux::bounded_sequence<S const>);
        static_assert(flux::sized_sequence<S const>);
        static_assert(std::same_as<flux::element_t<S const>, int&>);
        static_assert(std::same_as<flux::value_t<S const>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<S const>, int&&>);

        STATIC_CHECK(flux::size(seq) == 9);
        STATIC_CHECK(check_equal(seq, {0, 1, 2, 3, 4, 5, 6, 7, 8}));

        auto cur1 = seq.next(seq.first());
        auto cur2 = seq.prev(seq.last());

        STATIC_CHECK(seq.distance(cur1, cur2) == 7);

        // Make sure we're really multipass, not pretending
        auto cur = seq.find(4);
        (void) seq.next(cur);
        STATIC_CHECK(seq[cur] == 4);
    }

    // Const iteration works as expected
    {
        auto seq = flux::chain(std::array{1, 2, 3}, std::array{4, 5, 6});

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int&>);
        static_assert(std::same_as<flux::value_t<S>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);

        static_assert(flux::random_access_sequence<S const>);
        static_assert(flux::bounded_sequence<S const>);
        static_assert(flux::sized_sequence<S const>);
        static_assert(std::same_as<flux::element_t<S const>, int const&>);
        static_assert(std::same_as<flux::value_t<S const>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<S const>, int const&&>);

        STATIC_CHECK(check_equal(std::as_const(seq), {1, 2, 3, 4, 5, 6}));
    }

    // Chaining single-pass sequences works as expected
    {
        int arr1[] = {0, 1, 2};
        int arr2[] = {3, 4, 5};

        auto seq = flux::chain(single_pass_only(flux::ref(arr1)),
                               single_pass_only(flux::ref(arr2)));

        using S = decltype(seq);

        static_assert(flux::sequence<S>);
        static_assert(!flux::multipass_sequence<S>);

        STATIC_CHECK(check_equal(seq, {0, 1, 2, 3, 4, 5}));
    }

    // Non-const-iterable sequences can be chained
    {
        int arr1[] = {0, 1, 2};
        int arr2[] = {3, 4, 5};
        auto yes = [](int) { return true; };

        auto seq = flux::chain(flux::filter(flux::ref(arr1), yes), flux::filter(flux::ref(arr2), yes));

        using S = decltype(seq);

        static_assert(flux::bidirectional_sequence<S>);

        STATIC_CHECK(check_equal(seq, {0, 1, 2, 3, 4, 5}));
    }

    // Reverse works when all chained sequences are bidir + bounded
    {
        int arr1[] = {0, 1, 2};
        int arr2[] = {3, 4, 5};

        auto seq = flux::chain(flux::ref(arr1), flux::ref(arr2)).reverse();

        STATIC_CHECK(check_equal(seq, {5, 4, 3, 2, 1, 0}));
    }

    // Empty sequences are ignored as they should be
    {
        int arr1[] = {0, 1, 2};
        std::array arr2 = {3, 4, 5};

        auto seq = flux::chain(flux::ref(arr1),
                               flux::empty<int const>,
                               flux::ref(arr2),
                               flux::empty<int const>,
                               std::array{6, 7, 8});

        STATIC_CHECK(flux::size(seq) == 9);
        STATIC_CHECK(check_equal(seq, {0, 1, 2, 3, 4, 5, 6, 7, 8}));

        auto seq2 = flux::chain(flux::empty<int>, flux::empty<int>, flux::empty<int>);
        STATIC_CHECK(seq2.size() == 0);
        STATIC_CHECK(seq2.is_last(seq2.first()));
    }

    // We can sort across RA sequences
    {
        int arr1[] = {9, 8, 7};
        std::array arr2 = {6, 5, 4};

        auto seq = flux::chain(flux::mut_ref(arr1),
                               flux::empty<int>,
                               flux::mut_ref(arr2),
                               flux::empty<int>,
                               std::array{3, 2, 1});

        std::ranges::sort(seq);

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }

    return true;
}
static_assert(test_chain());

}

TEST_CASE("chain")
{
    bool result = test_chain();
    REQUIRE(result);
}