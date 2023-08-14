
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <flux.hpp>

#include <cassert>
#include <iostream>
#include <vector>

int main()
{
    std::array nums{1, 1, 2, 3, 3, 2, 2};

    // The adjacent_filter adaptor applies the given predicate to each pair
    // of elements in the sequence, and if the predicate returns false then
    // the second element of the pair is discarded
    auto filtered1 = flux::adjacent_filter(nums, std::less{});
    assert(flux::equal(filtered1, std::array{1, 2, 3}));

    // For the common case of removing adjacent equal elements, Flux provides
    // the dedup() function as shorthand for adjacent_filter(std::not_equal_to{})
    auto filtered2 = flux::dedup(nums);
    assert(flux::equal(filtered2, std::array{1, 2, 3, 2}));

    // We can use adjacent_filter with a custom comparator as well
    auto compare = [](auto p1, auto p2) { return p1.first != p2.first; };
    std::pair<int, int> pairs[] = {{1, 2}, {1, 3}, {1, 4}, {2, 5}, {2, 6}};

    auto filtered3 = flux::adjacent_filter(flux::ref(pairs), compare);
    assert(flux::equal(filtered3,
                       std::array{std::pair{1, 2}, std::pair{2, 5}}));
}