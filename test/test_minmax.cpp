
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>

#include "test_utils.hpp"

namespace {

struct IntPair {
    int a, b;
    friend bool operator==(IntPair const&, IntPair const&) = default;
};


constexpr bool test_min()
{
    // Empty range -> no min value
    {
        auto opt = flux::min(flux::empty<int>);
        STATIC_CHECK(!opt.has_value());
    }

    // Basic min works as expected
    {
        STATIC_CHECK(flux::min(std::array{5, 4, 3, 2, 1}).value() == 1);

        STATIC_CHECK(flux::from(std::array{5, 4, 3, 2, 1}).min().value() == 1);
    }

    // Can use custom comparator and projection
    {
        IntPair arr[] = { {1, 2}, {3, 4}, {5, 6}};
        STATIC_CHECK(flux::min(arr, flux::proj(std::greater<>{}, &IntPair::a)).value() == IntPair{5, 6});
    }

    // If several elements are equally minimal, returns the first
    {
        IntPair arr[] = { {1, 2}, {1, 3}, {1, 4}};
        STATIC_CHECK(flux::min(arr, flux::proj(std::ranges::less{}, &IntPair::a))->b == 2);
    }

    return true;
}
static_assert(test_min());

constexpr bool test_max()
{
    // Empty range -> no max value
    {
        auto opt = flux::max(flux::empty<int>);
        STATIC_CHECK(!opt.has_value());
    }

    // Basic max works as expected
    {
        STATIC_CHECK(flux::max(std::array{5, 4, 3, 2, 1}).value() == 5);

        STATIC_CHECK(flux::from(std::array{5, 4, 3, 2, 1}).max().value() == 5);
    }

    // Can use custom comparator and projection
    {
        IntPair arr[] = { {1, 2}, {3, 4}, {5, 6}};
        STATIC_CHECK(flux::max(arr, flux::proj(std::greater<>{}, &IntPair::a)).value() == IntPair{1, 2});
    }

    // If several elements are equally maximal, returns the last
    {
        IntPair arr[] = { {1, 2}, {1, 3}, {1, 4}};
        STATIC_CHECK(flux::max(arr, flux::proj(std::ranges::less{}, &IntPair::a))->b == 4);
    }

    return true;
}
static_assert(test_max());

constexpr bool test_minmax()
{
    // Empty range -> no minmax
    {
        auto opt = flux::minmax(flux::empty<int>);
        STATIC_CHECK(!opt.has_value());
    }

    // Basic minmax works as expected
    {
        auto result = flux::minmax(std::array{5, 4, 3, 2, 1}).value();
        STATIC_CHECK(result.min == 1);
        STATIC_CHECK(result.max == 5);

        result = flux::from(std::array{5, 4, 3, 2, 1}).minmax(std::greater{}).value();
        STATIC_CHECK(result.min == 5);
        STATIC_CHECK(result.max == 1);
    }

    // Can use custom comparator and projection
    {
        IntPair arr[] = { {1, 2}, {3, 4}, {5, 6}};
        auto result = flux::minmax(arr, flux::proj(std::greater<>{}, &IntPair::a)).value();
        STATIC_CHECK(result.min == IntPair{5, 6});
        STATIC_CHECK(result.max == IntPair{1, 2});
    }

    // If several elements are equally minimal/maximal, returns the first/last resp.
    {
        IntPair arr[] = { {1, 2}, {1, 3}, {1, 4}};
        auto result = flux::minmax(arr, flux::proj(std::ranges::less{}, &IntPair::a)).value();
        STATIC_CHECK(result.min == IntPair{1, 2});
        STATIC_CHECK(result.max == IntPair{1, 4});
    }

    return true;
}
static_assert(test_minmax());

}
