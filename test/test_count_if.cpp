
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

constexpr auto is_even = [](int i) { return i % 2 == 0; };

constexpr bool test_count_if()
{
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        STATIC_CHECK(flux::count_if(arr, is_even) == 5);

        STATIC_CHECK(flux::from(arr).count_if(is_even) == 5);
    }

    {
        S arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        STATIC_CHECK(flux::count_if(arr, is_even, &S::i));

        STATIC_CHECK(flux::from(arr).count_if(is_even, &S::get));
    }

    return true;
}
static_assert(test_count_if());

}

TEST_CASE("count_if")
{
    bool result = test_count_if();
    REQUIRE(result);
}