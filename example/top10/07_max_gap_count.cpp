
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
 * https://theweeklychallenge.org/blog/perl-weekly-challenge-198/
 *
 * Also discussed on ADSP episode 116 (https://adspthepodcast.com/2023/02/03/Episode-116.html)
 */

#include <flux.hpp>

// std::abs is not constexpr in C++20
auto const c_abs = [](std::signed_integral auto i) { return i < 0 ? -i : i; };

namespace version1 {

// Sort + two passes
auto const max_gap_count = [](std::vector<int> nums)
{
    flux::sort(nums);

    auto diffs = flux::ref(nums).pairwise_map([](int a, int b) { return c_abs(a - b); });

    return diffs.count_eq(diffs.max().value_or(0));
};

static_assert(max_gap_count({2, 5, 8, 1}) == 2);
static_assert(max_gap_count({3, 6, 9, 1}) == 2);
static_assert(max_gap_count({10}) == 0);

}

namespace version2 {

// Sort + one pass
auto const max_gap_count = [](std::vector<int> nums)
{
    struct max_count {
        int value;
        int count;
    };

    flux::sort(nums);
    return flux::ref(nums)
            .pairwise_map([](int a, int b) { return c_abs(a - b); })
            .fold([](max_count max, int i) {
                if (i > max.value) {
                    max = max_count{i, 1};
                } else if (i == max.value ){
                    ++max.count;
                }
                return max;
            }, max_count{})
            .count;
};

static_assert(max_gap_count({2, 5, 8, 1}) == 2);
static_assert(max_gap_count({3, 6, 9, 1}) == 2);
static_assert(max_gap_count({10}) == 0);

}

int main() {}
