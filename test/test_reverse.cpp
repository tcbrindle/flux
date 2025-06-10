
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

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

// Regression test for #182
// https://github.com/tcbrindle/flux/issues/182
constexpr bool issue_182()
{
    auto seq = flux::iota(1,4)
                   .drop(2)
                   .reverse().drop(5).reverse()
                   .filter([](int i) { return i&1; })
                   .chunk(4);

    return seq.is_empty();
}
static_assert(issue_182());

}

TEST_CASE("reverse")
{
    bool result = test_reverse();
    REQUIRE(result);

    result = issue_52();
    REQUIRE(result);

    result = issue_143();
    REQUIRE(result);

    result = issue_182();
    REQUIRE(result);

    {
        auto list = std::list{0, 1, 2, 3, 4};
        auto rlist = flux::reverse(std::move(list));

        using R = decltype(rlist);

        static_assert(flux::iterable<R>);
        static_assert(flux::reverse_iterable<R>);
        static_assert(flux::sized_iterable<R>);

        REQUIRE(check_equal(rlist, {4, 3, 2, 1, 0}));
    }
}