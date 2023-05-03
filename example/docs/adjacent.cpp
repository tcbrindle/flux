
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <flux.hpp>

#include <cassert>
#include <iostream>
#include <vector>

int main()
{
    std::vector vec{1, 2, 3, 4, 5};

    std::vector<int> sums;

    // adjacent<3> yields 3-tuples which we can destructure
    for (auto [a, b, c] : flux::adjacent<3>(std::move(vec))) {
        sums.push_back(a + b + c);
    }

    assert((sums == std::vector{1 + 2 + 3,
                                2 + 3 + 4,
                                3 + 4 + 5}));
}