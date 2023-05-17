
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
 * Given an unsorted array of integers nums, return the length of the longest
 * continuous increasing subsequence (i.e. subarray). The subsequence must be
 * strictly increasing.
 *
 * https://leetcode.com/problems/longest-continuous-increasing-subsequence/
 */

#include <flux.hpp>

auto const lcis = [](std::initializer_list<int> nums)
{
    return flux::ref(nums)
            .chunk_by(std::less{})
            .map(flux::count)
            .max()
            .value_or(0);
};

static_assert(lcis({}) == 0);
static_assert(lcis({1, 3, 5, 4, 7}) == 3); // [1, 3, 5]
static_assert(lcis({2, 2, 2, 2, 2}) == 1); // [2] (must be strictly increasing)

int main() {}