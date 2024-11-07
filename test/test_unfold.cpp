
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <utility>

#include "test_utils.hpp"

namespace {

constexpr bool test_unfold()
{
    // Basic unfold
    {
        auto seq = flux::unfold([](int i) { return ++i; }, 0);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(flux::infinite_sequence<S>);
        static_assert(not flux::sized_iterable<S>);

        STATIC_CHECK(check_equal(flux::take(seq, 10), flux::ints().take(10)));
    }

    // unfold -> take is a finite sequence
    {
        auto seq = flux::unfold([](int i) { return ++i; }, 0).take(10);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(flux::sized_iterable<S>);

        STATIC_CHECK(flux::size(seq) == 10);
        STATIC_CHECK(check_equal(seq, flux::ints().take(10)));
    }

    // unfold can be used to implement repeat()
    {
        auto repeat = flux::unfold(std::identity{}, std::string_view("hello"));

        STATIC_CHECK(check_equal(flux::take(repeat, 3), {"hello", "hello", "hello"}));
    }

    // unfold works with mutable stateful lambdas
    {
        auto fib = flux::unfold([next = 1](int cur) mutable {
            return std::exchange(next, cur + next);
        }, 0).take(10);

        STATIC_CHECK(check_equal(fib, {0, 1, 1, 2, 3, 5, 8, 13, 21, 34}));
    }

    // internal iteration works as expected
    {
        auto seq = flux::unfold([](int i) { return ++i; }, 0);

        auto cur = seq.find(5);

        STATIC_CHECK(seq[cur] == 5);
        seq.inc(cur);
        STATIC_CHECK(seq[cur] == 6);
    }

    return true;
}
static_assert(test_unfold());

}

TEST_CASE("unfold")
{
    bool res = test_unfold();
    REQUIRE(res);
}