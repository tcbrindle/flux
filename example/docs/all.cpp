
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "assert.hpp"
#include <vector>

#include "../import_or_include_flux.hpp"

int main()
{
    auto is_small = [](int i) { return i < 10; };

    std::vector<int> vec{1, 2, 3, 4, 5};

    // Check whether every element is small
    bool all_small = flux::all(vec, is_small);
    assert(all_small);
}