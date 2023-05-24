
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux/op/count.hpp>
#include <flux/op/take_while.hpp>

#include "test_utils.hpp"


namespace {

struct S {

    constexpr S(int i) : i(i) {}

    constexpr int get() const { return i; }

    int i;
};

constexpr bool test_count()
{
    {
        int arr[] = {1, 2, 3, 4, 5};
        STATIC_CHECK(flux::count(arr) == 5);

        auto seq = flux::take_while(flux::ref(arr), [](int) { return true; });
        static_assert(not flux::sized_sequence<decltype(seq)>);

        STATIC_CHECK(flux::count(seq) == 5);
    }

    {
        int arr[] = {1, 2, 3, 4, 5};
        STATIC_CHECK(flux::ref(arr).count() == 5);

        auto seq = flux::take_while(flux::ref(arr), [](int) { return true; });
        static_assert(not flux::sized_sequence<decltype(seq)>);

        STATIC_CHECK(seq.count() == 5);
    }

    {
        int arr[] = {1, 2, 2, 2, 3, 4, 5};
        STATIC_CHECK(flux::count_eq(arr, 2) == 3);
        STATIC_CHECK(flux::count_eq(arr, 99) == 0);

        auto seq = flux::ref(arr);
        STATIC_CHECK(seq.count_eq(2) == 3);
        STATIC_CHECK(seq.count_eq(99) == 0);
    }

    return true;
}
static_assert(test_count());

}

TEST_CASE("count")
{
    bool result = test_count();
    REQUIRE(result);
}