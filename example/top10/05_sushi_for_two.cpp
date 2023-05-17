
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
 * https://codeforces.com/contest/1138/problem/A
 */

#include <flux.hpp>

auto const sushi_for_two = [](std::initializer_list<int> sushi)
{
    return 2 * flux::ref(sushi)
                .chunk_by(std::equal_to{})
                .map(flux::count)
                .pairwise_map(std::ranges::min)
                .max()
                .value_or(0);
};

static_assert(sushi_for_two({}) == 0);
static_assert(sushi_for_two({2, 2, 2, 1, 1, 2, 2}) == 4);
static_assert(sushi_for_two({1, 2, 1, 2, 1, 2}) == 2);
static_assert(sushi_for_two({2, 2, 1, 1, 1, 2, 2, 2, 2}) == 6);

int main() {}