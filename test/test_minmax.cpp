
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

namespace {

constexpr bool test_min()
{
    {
        STATIC_CHECK(flux::min(std::array{5, 4, 3, 2, 1}).value() == 1);
    }

    return true;
}
static_assert(test_min());

constexpr bool test_max()
{
    {
        STATIC_CHECK(flux::max(std::array{5, 4, 3, 2, 1}).value() == 5);
    }

    return true;
}
static_assert(test_max());

constexpr bool test_minmax()
{
    {
        auto [min, max] = flux::minmax(std::array{5, 4, 3, 2, 1}).value();

        STATIC_CHECK(min == 1);
        STATIC_CHECK(max == 5);
    }

    return true;
}
static_assert(test_minmax());

}
