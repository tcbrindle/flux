
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>
#include <list>

#include "test_utils.hpp"

namespace {

constexpr bool test_reverse()
{
    {
        int arr[] = {0, 1, 2, 3, 4};

        auto reversed = flux::reverse(flux::ref(arr));

        using R = decltype(reversed);

        static_assert(flux::random_access_sequence<R>);
        static_assert(not flux::contiguous_sequence<R>);
        static_assert(flux::bounded_sequence<R>);
        static_assert(flux::sized_sequence<R>);

        static_assert(flux::random_access_sequence<R const>);
        static_assert(not flux::contiguous_sequence<R const>);
        static_assert(flux::bounded_sequence<R const>);
        static_assert(flux::sized_sequence<R const>);

        STATIC_CHECK(flux::size(reversed) == 5);
        STATIC_CHECK(check_equal(reversed, {4, 3, 2, 1, 0}));
    }

    {
        auto reversed = flux::from(std::array{0, 1, 2, 3, 4}).reverse();

        using R = decltype(reversed);

        static_assert(flux::random_access_sequence<R>);
        static_assert(not flux::contiguous_sequence<R>);
        static_assert(flux::bounded_sequence<R>);
        static_assert(flux::sized_sequence<R>);

        static_assert(flux::random_access_sequence<R const>);
        static_assert(not flux::contiguous_sequence<R const>);
        static_assert(flux::bounded_sequence<R const>);
        static_assert(flux::sized_sequence<R const>);

        STATIC_CHECK(flux::size(reversed) == 5);
        STATIC_CHECK(check_equal(reversed, {4, 3, 2, 1, 0}));
    }

    {
        std::array arr{0, 1, 2, 3, 4};

        auto seq = flux::ref(arr).reverse().reverse();

        using S = decltype(seq);

        static_assert(flux::contiguous_sequence<S>);
        static_assert(std::same_as<S, decltype(flux::ref(arr))>);

        STATIC_CHECK(seq.data() == arr.data());
    }

    {
        std::array arr{0, 1, 2, 3, 4};

        auto seq = flux::ref(arr).reverse().reverse().reverse();

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(std::same_as<S, decltype(flux::reverse(flux::ref(arr)))>);

        STATIC_CHECK(check_equal(seq, {4, 3, 2, 1, 0}));
    }

    return true;
}
static_assert(test_reverse());

// Regression test for #52
// https://github.com/tcbrindle/flux/issues/52
constexpr bool issue_52()
{
    std::string_view in = "   abc   ";
    std::string_view out = "abc";

    auto is_space = flux::pred::in(' ', '\t', '\n', '\r');

    auto seq = flux::drop_while(in, is_space)
                    .reverse()
                    .drop_while(is_space)
                    .reverse();

    STATIC_CHECK(check_equal(seq, out));

    return true;
}
static_assert(issue_52());

// Regression test for #143
// https://github.com/tcbrindle/flux/issues/143
constexpr bool issue_143()
{
    struct Int {
        int i;
        constexpr int get() { return i; } // not const
    };

    std::array<Int, 3> arr{Int{1}, {2}, {3}};

    int sum = 0;
    for (int i : flux::map(arr, &Int::get).reverse()) {
        sum += i;
    }

    STATIC_CHECK(sum == 6);

    return true;
}
static_assert(issue_143());


}

TEST_CASE("reverse")
{
    bool result = test_reverse();
    REQUIRE(result);

    result = issue_52();
    REQUIRE(result);

    {
        auto list = std::list{0, 1, 2, 3, 4};
        auto rlist = flux::reverse(flux::from_range(list));

        using R = decltype(rlist);

        static_assert(flux::regular_cursor<flux::cursor_t<R>>);
        static_assert(flux::bidirectional_sequence<R>);
        static_assert(not flux::random_access_sequence<R>);
        static_assert(flux::sized_sequence<R>);
        static_assert(flux::bounded_sequence<R>);

        REQUIRE(check_equal(rlist, {4, 3, 2, 1, 0}));
    }
}