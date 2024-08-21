
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
    std::array nums{1, 2, 3};

    // cartesian_power_map<N> takes a sequence and a callable of N arguments
    // Here we are using N=2 and the binary function object std::plus
    auto sums = flux::cartesian_power_map<2>(nums, std::plus{}).to<std::vector<int>>();

    assert((sums == std::vector<int>{1 + 1, 1 + 2, 1 + 3,
                                     2 + 1, 2 + 2, 2 + 3,
                                     3 + 1, 3 + 2, 3 + 3}));
}