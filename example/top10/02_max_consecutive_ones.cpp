
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
 * Given a binary array nums, return the maximum number of consecutive 1's in the array.
 *
 * https://leetcode.com/problems/max-consecutive-ones/
 */

#include <flux.hpp>

namespace version1 {

auto const mco = [](std::initializer_list<int> nums)
{
    return flux::ref(nums)
            .chunk_by(std::equal_to{})
            .map(flux::sum)
            .max()
            .value_or(0);
};

static_assert(mco({1,1,0,1,1,1}) == 3);
static_assert(mco({1,0,1,1,0,1}) == 2);

}

namespace version2 {

auto const mco = [](std::initializer_list<int> nums)
{
    return flux::ref(nums)
            .scan([](int count, int i) { return i * (count + i); })
            .max()
            .value_or(0);
};

static_assert(mco({1,1,0,1,1,1}) == 3);
static_assert(mco({1,0,1,1,0,1}) == 2);

}

int main() {}