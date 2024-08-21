
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include "assert.hpp"
#include <array>

int main()
{
    // We can compute the triangular numbers using scan()
    std::array const ints{1, 2, 3, 4, 5};

    // Note that unlike prescan(), the initial value for scan() may be omitted
    // (here defaulting to int{}), and the resulting sequence has the same number
    // of elements as the original
    auto tri_nums = flux::scan(ints, std::plus{});
    assert(flux::equal(tri_nums, std::array{1, 3, 6, 10, 15}));
}