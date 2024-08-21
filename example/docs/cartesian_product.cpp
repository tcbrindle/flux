
// Copyright (c) 2024 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <flux.hpp>

#include "assert.hpp"
#include <iostream>
#include <vector>

using namespace std::string_view_literals;

int main()
{
    std::string_view str = "abc";
    std::array nums = {1, 2, 3};

    // flux::cartesian_product(str, nums) yields all combinations of elements from
    // the two sequences as a tuple<char, int>
    auto pairs = flux::cartesian_product(str, nums).to<std::vector>();

    using P = std::tuple<char, int>;

    assert((pairs == std::vector{P{'a', 1}, P{'a', 2}, P{'a', 3},
                                 P{'b', 1}, P{'b', 2}, P{'b', 3},
                                 P{'c', 1}, P{'c', 2}, P{'c', 3}}));

    // cartesian_product can take an arbitrary number of sequence arguments
    // The number of elements is the product of the sizes of the input sequences
    // It is bidirectional and random-access if all the input sequences
    // satisfy these concepts
    auto seq = flux::cartesian_product("xy"sv,
                                       std::array{1.0, 2.0},
                                       std::vector{111, 222}).reverse();

    using T = std::tuple<char, double, int>;

    assert(flux::equal(seq, std::vector<T>{{'y', 2.0, 222}, {'y', 2.0, 111},
                                           {'y', 1.0, 222}, {'y', 1.0, 111},
                                           {'x', 2.0, 222}, {'x', 2.0, 111},
                                           {'x', 1.0, 222}, {'x', 1.0, 111}}));
}