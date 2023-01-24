
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux/op/fill.hpp>
#include <flux/op/take.hpp>
#include <flux/source/empty.hpp>
#include <flux/source/single.hpp>

#include <array>

#include "test_utils.hpp"

namespace {

constexpr bool test_fill()
{
    // Basic fill()
    {
        std::array<int, 5> arr{};

        flux::fill(arr, 1);

        STATIC_CHECK(check_equal(arr, {1, 1, 1, 1, 1}));
    }

    // fill and adapted sequence
    {
        std::array<int, 5> arr{};

        flux::take(flux::ref(arr), 3).fill(1);

        STATIC_CHECK(check_equal(arr, {1, 1, 1, 0, 0}));
    }

    // single-pass sequences can be filled
    {
        std::array<int, 5> arr{};

        single_pass_only(flux::from(arr)).fill(1);

        STATIC_CHECK(check_equal(arr, {1, 1, 1, 1, 1}));
    }

    // empty sequences can be "filled"
    {
        auto e = flux::empty<int>;
        flux::fill(e, 99);
    }

    // single sequences can be filled
    {
        auto s= flux::single(0);

        flux::fill(s, short{1});

        STATIC_CHECK(s.value() == 1);
    }

    return true;
}
static_assert(test_fill());

}

TEST_CASE("fill")
{
    bool result = test_fill();
    REQUIRE(result);
}