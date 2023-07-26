
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
 * There are n buildings in a line. You are given an integer array heights of
 * size n that represents the heights of the buildings in the line.
 *
 * The ocean is to the right of the buildings. A building has an ocean view if
 * the building can see the ocean without obstructions. Formally, a building has
 * an ocean view if all the buildings to its right have a smaller height.
 *
 * https://leetcode.ca/all/1762.html
 */

#include <flux.hpp>

#include <algorithm>
#include <cassert>
#include <vector>

using index_vec = std::vector<flux::index_t>;

auto const ocean_view = [](std::vector<int> const& heights) -> index_vec
{
    auto indices = flux::zip(flux::ref(heights).cursors().reverse(),
                             flux::ref(heights).reverse().prescan(std::ranges::max, 0))
        .filter([&](auto pair) { return heights[pair.first] > pair.second; })
        .map([](auto pair) { return pair.first; })
        .to<index_vec>();

    flux::inplace_reverse(indices);

    return indices;
};

int main()
{
    assert(ocean_view({4,2,3,1}) == index_vec({0,2,3}));
    assert(ocean_view({4,3,2,1}) == index_vec({0,1,2,3}));
    assert(ocean_view({1,3,2,4}) == index_vec{3});
    assert(ocean_view({2,2,2,2}) == index_vec{3});
}