
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include <array>

#include "test_utils.hpp"

namespace {

struct IntPair {
    int a, b;
    friend bool operator==(IntPair const&, IntPair const&) = default;
};


constexpr bool test_find_min()
{
    // Empty range -> no min value
    {
        auto seq = flux::empty<int>;
        auto cur = flux::find_min(seq);
        STATIC_CHECK(seq.is_last(cur));
    }

    // Basic min works as expected
    {
        auto arr = std::array{5, 4, 3, 2, 1};

        STATIC_CHECK(flux::read_at(arr, flux::find_min(arr)) == 1);

        auto ref = flux::ref(arr);
        STATIC_CHECK(ref[ref.find_min()] == 1); // much better!
    }

    // Can use custom comparator and projection
    {
        IntPair arr[] = { {1, 2}, {3, 4}, {5, 6}};

        auto cur = flux::find_min(arr, flux::proj(std::greater{}, &IntPair::a));

        STATIC_CHECK(not flux::is_last(arr, cur));
        STATIC_CHECK(flux::read_at(arr, cur) == IntPair{5, 6});

    }

    // If several elements are equally minimal, returns the first
    {
        IntPair arr[] = { {1, 2}, {1, 3}, {1, 4}};

        auto cur = flux::find_min(arr, flux::proj(std::less{}, &IntPair::a));

        STATIC_CHECK(not flux::is_last(arr, cur));
        STATIC_CHECK(flux::read_at(arr, cur).b == 2);
    }

    return true;
}
static_assert(test_find_min());

constexpr bool test_find_max()
{
    // Empty range -> no max value
    {
        auto seq = flux::filter(std::array{2, 4, 6, 8, 10}, flux::pred::odd);
        auto max = seq.find_max();
        STATIC_CHECK(seq.is_last(max));
    }

    // Basic max works as expected
    {
        auto seq = flux::from(std::array{5, 4, 3, 2, 1});
        auto cur = seq.find_max();
        STATIC_CHECK(cur == 0);
        STATIC_CHECK(seq[cur] == 5);
    }

    // Can use custom comparator and projection
    {
        IntPair arr[] = { {1, 2}, {3, 4}, {5, 6}};

        auto cur = flux::find_max(arr, flux::proj(std::greater{}, &IntPair::a));

        STATIC_CHECK(flux::read_at(arr, cur) == IntPair{1, 2});
    }

    // If several elements are equally maximal, returns the last
    {
        IntPair arr[] = { {1, 2}, {1, 3}, {1, 4}};

        auto cur = flux::find_max(arr, flux::proj(std::less{}, &IntPair::b));

        STATIC_CHECK(flux::read_at(arr, cur).b == 4);
    }

    return true;
}
static_assert(test_find_max());

constexpr bool test_find_minmax()
{
    // Empty range -> no minmax
    {
        auto seq = flux::filter(std::array{2, 4, 6, 8, 10}, flux::pred::odd);
        auto [min, max] = seq.find_minmax();
        STATIC_CHECK(seq.is_last(min));
        STATIC_CHECK(seq.is_last(max));
    }

    // Basic minmax works as expected
    {
        auto seq = flux::from(std::array{5, 4, 3, 2, 1});

        auto result = seq.find_minmax();

        STATIC_CHECK(seq[result.min] == 1);
        STATIC_CHECK(seq[result.max] == 5);
    }

    // Can use custom comparator and projection
    {
        IntPair arr[] = { {1, 2}, {3, 4}, {5, 6}};

        auto result = flux::find_minmax(arr, flux::proj(std::greater<>{}, &IntPair::a));


        STATIC_CHECK(flux::read_at(arr, result.min) == IntPair{5, 6});
        STATIC_CHECK(flux::read_at(arr, result.max) == IntPair{1, 2});
    }

    // If several elements are equally minimal/maximal, returns the first/last resp.
    {
        IntPair arr[] = { {1, 2}, {1, 3}, {1, 4}};
        auto [min, max] = flux::find_minmax(arr, flux::proj(std::ranges::less{}, &IntPair::a));

        STATIC_CHECK(flux::read_at(arr, min) == IntPair{1, 2});
        STATIC_CHECK(flux::read_at(arr, max) == IntPair{1, 4});
    }

    return true;
}
static_assert(test_find_minmax());

}
