
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include "assert.hpp"
#include <array>

int main()
{
    // We can compute the triangular numbers using prescan()
    std::array const ints{1, 2, 3, 4, 5};

    // Note that unlike scan(), the initial value for prescan() is required, and
    // is the first element of the resulting sequence, which has one more element
    // than the input
    auto tri_nums = flux::prescan(ints, std::plus{}, 0);
    assert(flux::equal(tri_nums, std::array{0, 1, 3, 6, 10, 15}));
}