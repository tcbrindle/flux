
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "assert.hpp"
#include <array>

#include "../import_or_include_flux.hpp"

int main()
{
    // We can compute the triangular numbers using scan_first()
    std::array const ints{1, 2, 3, 4, 5};

    // Note that unlike scan(), scan_first() never takes an initial value,
    // and instead the first element of the resulting sequence is the same as
    // the first element of the underlying sequence: the fold operation is only
    // applied to subsequent elements.
    auto tri_nums = flux::scan_first(ints, flux::num::add);
    assert(flux::equal(tri_nums, std::array{1, 3, 6, 10, 15}));
}