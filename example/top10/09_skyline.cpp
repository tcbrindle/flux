
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
 * Given an integer list of skyscraper heights from left to right, calculate how
 * many buildings can be seen by an observer standing at the left-most position.
 *
 * (Ignoring trigonometry...)
 *
 * https://youtu.be/6AWSPC6qQB4?t=560
 */

#include <flux.hpp>

auto const skyline = [](std::initializer_list<int> heights)
{
    auto h = flux::ref(heights);

    return 1 + flux::zip(flux::drop(h, 1), flux::scan(h, flux::cmp::max))
                   .count_if([](auto p) { return p.first > p.second; });
};

static_assert(skyline({5, 5, 2, 10, 3, 15, 10}) == 3);

int main() {}