
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cassert>
#include <vector>

int main()
{
    auto is_small = [](int i) { return i < 10; };

    std::vector<int> vec{1, 2, 3, 4, 5};

    // Check whether every element is small
    bool all_small = flux::all(vec, is_small);
    assert(all_small);
}