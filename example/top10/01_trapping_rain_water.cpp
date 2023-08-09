
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
 * Given n non-negative integers representing an elevation map where the width
 * of each bar is 1, compute how much water it can trap after raining.
 *
 * https://leetcode.com/problems/trapping-rain-water/
 *
 */

#include <flux.hpp>

#include <algorithm>

auto const rain_water = [](std::initializer_list<int> heights)
{
    // Find the index of the maximum height
    flux::cursor auto max_idx = flux::find_max(heights);

    // Split the input sequence into two at the max position
    auto left = flux::slice(heights, flux::first(heights), max_idx);
    auto right = flux::slice(heights, max_idx, flux::last(heights));

    // To calculate the trapped rain water for each half, we sum up the
    // difference between each element and the maximum seen up to that point
    auto trapped = [](flux::sequence auto seq) {
        return flux::zip(flux::scan(seq, flux::cmp::max), seq)
                .map(flux::unpack(std::minus{}))
                .sum();
    };

    // The final answer is the total of the trapped rain water reading
    // left-to-right for the left half, and right-to-left for the right half
    return trapped(left) + trapped(flux::reverse(right));
};

static_assert(rain_water({0,1,0,2,1,0,1,3,2,1,2,1}) == 6);
static_assert(rain_water({4,2,0,3,2,5}) == 9);

int main() {}