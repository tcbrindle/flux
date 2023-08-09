
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
 * Given an integer array nums, find the subarray with the largest sum, and
 * return its sum.
 *
 * https://leetcode.com/problems/maximum-subarray/
 *
 */

#include <flux.hpp>

auto const kadanes = [](std::initializer_list<int> nums)
{
    return flux::ref(nums)
            .scan([](int sum, int i) { return std::max(i, sum + i); })
            .max()
            .value_or(0);
};

static_assert(kadanes({-2, 1, -3, 4, -1, 2, 1, -5, 4}) == 6);
static_assert(kadanes({1}) == 1);
static_assert(kadanes({5, 4, -1, 7, 8}) == 23);

int main() {}