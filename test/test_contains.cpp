
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux/op/contains.hpp>

#include "test_utils.hpp"

#include <array>

namespace {

struct Test {
    constexpr Test(int i) : i(i) {}

    constexpr int get() { return i; }

    int i;
};

template <bool = true>
constexpr bool test_contains()
{
    // Basic contains
    {
        int arr[] = {0, 1, 2, 3, 4};

        STATIC_CHECK(flux::contains(arr, 3));
        STATIC_CHECK(not flux::contains(arr, 99));

        std::string_view sv = "Hello World";

        auto seq = flux::from(sv);

        STATIC_CHECK(seq.contains(' '));
        STATIC_CHECK(not seq.contains('Z'));
    }

    return true;
}
static_assert(test_contains());

}

TEST_CASE("contains")
{
    bool result = test_contains<false>();
    REQUIRE(result);
}