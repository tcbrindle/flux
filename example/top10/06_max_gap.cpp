
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
 * https://leetcode.com/problems/maximum-gap/
 *
 * Note that the original problem specifies that the solution should have O(N)
 * runtime and use O(N) extra space. We instead use an O(NlogN) solution as
 * discussed on ADSP episode 115 (https://adspthepodcast.com/2023/02/03/Episode-115.html)
 * The NlogN version still passes the time limit constraints on leetcode, however.
 */

#include <flux.hpp>

namespace {

// std::abs is not constexpr in C++20
auto const abs = [](std::signed_integral auto i) { return i < 0 ? -i : i; };

auto const max_gap = [](std::vector<int> nums)
{
    flux::sort(nums);
    return flux::ref(nums)
            .pairwise_map([](int a, int b) { return abs(a - b); })
            .max()
            .value_or(0);
};

}

int main()
{
    FLUX_ASSERT(max_gap({3, 6, 9, 1}) == 3);
    FLUX_ASSERT(max_gap({10}) == 0);
}
