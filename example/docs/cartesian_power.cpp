
// Copyright (c) 2024 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <flux.hpp>

#include "assert.hpp"
#include <iostream>
#include <vector>

int main()
{
    std::string_view str = "ab";
    std::vector<std::string> strings;

    // cartesian_power<3> gives us 3-tuples which we can destructure
    // The total number of elements in this case is size(str)^3 = 8
    for (auto [x, y, z] : flux::cartesian_power<3>(str)) {
        strings.push_back(std::string{x, y, z});
    }

    assert((strings == std::vector<std::string>{"aaa", "aab", "aba", "abb",
                                                "baa", "bab", "bba", "bbb"}));
}