
// Copyright (c) 2024 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <flux.hpp>

#include "assert.hpp"
#include <iostream>
#include <vector>

int main()
{
    std::array big = {10000L, 20000L};
    std::array medium = {100.0f, 200.0f};
    std::array small = {1, 2};

    auto add3 = [](long a, float b, int c) { return double(a) + b + c; };

    // Note that because cartesian_product_map takes a variadic number of
    // sequences, the function argument goes first
    auto vec = flux::cartesian_product_map(add3, big, medium, small)
                   .to<std::vector>();

    assert((vec == std::vector<double>{10101.0, 10102.0,
                                       10201.0, 10202.0,
                                       20101.0, 20102.0,
                                       20201.0, 20202.0}));
}