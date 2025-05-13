
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
 * Given an integer array arr, return true if there are three consecutive odd
 * numbers in the array. Otherwise, return false.
 *
 * https://leetcode.com/problems/three-consecutive-odds/
 */

#include <flux.hpp>

namespace version1 {

auto const tco = [](std::initializer_list<int> nums)
{
    int odd_count = 0;
    auto idx = flux::seq_for_each_while(nums, [&](int i) {
        if (i % 2 != 0) {
            ++odd_count;
        } else {
            odd_count = 0;
        }
        return odd_count < 3; // true => keep iterating
    });
    return !flux::is_last(nums, idx);
};

static_assert(tco({}) == false);
static_assert(tco({2, 6, 4, 1}) == false);
static_assert(tco({1, 2, 34, 3, 4, 5, 7, 23, 12}) == true);

}

namespace version2 {

auto const tco = [](std::initializer_list<int> nums)
{
    auto all_true = [](auto... b) { return (b && ...); };
    return flux::ref(nums)
                .map(flux::pred::odd)
                .adjacent_map<3>(all_true)
                .any(flux::pred::id);
};

static_assert(tco({}) == false);
static_assert(tco({2, 6, 4, 1}) == false);
static_assert(tco({1, 2, 34, 3, 4, 5, 7, 23, 12}) == true);

}

namespace version3 {

auto const tco = [](std::initializer_list<int> nums)
{
    return flux::ref(nums)
            .scan([](int count, int elem) { return (elem % 2 != 0) ? ++count : 0; })
            .any(flux::pred::geq(3));
};

static_assert(tco({}) == false);
static_assert(tco({2, 6, 4, 1}) == false);
static_assert(tco({1, 2, 34, 3, 4, 5, 7, 23, 12}) == true);

}

int main() {}