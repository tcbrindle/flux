
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

#include <cassert>
#include <vector>

using index_vec = std::vector<flux::index_t>;

// Alas, no nice way to do this with Flux adaptors (yet)
auto const ocean_view = [](std::vector<int> const& heights) -> index_vec
{
    int max_so_far = 0;
    index_vec indices;

    flux::ints(0, flux::size(heights)).reverse().for_each([&](auto idx){
        if (heights[idx] > max_so_far) {
            max_so_far = heights[idx];
            indices.push_back(idx);
        }
    });

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