
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cassert>
#include <vector>

int main()
{
    auto is_even = [](int i) { return i % 2 == 0; };

    std::vector<int> vec{1, 3, 4, 5, 7, 9};

    bool b = flux::any(vec, is_even);
    assert(b == true); // There is an even element
}