
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include "assert.hpp"
#include <iostream>
#include <vector>

// We can use the read_only_sequence concept to statically require a sequence
// whose elements are immutable
bool contains_a_two(flux::read_only_sequence auto&& seq)
{
    for (auto&& elem : seq) {
        if (elem == 2) { // What if we wrote `elem = 2` (assignment) by mistake?
            return true;
        }
    }
    return false;
}

int main()
{
    auto seq = flux::filter(std::vector{1, 2, 3, 4, 5}, flux::pred::even);

    // We cannot pass seq directly, as it yields mutable int& elements
    // contains_a_two(seq); // COMPILE ERROR

    // ...but we can use read_only() so that the sequence yields immutable
    // elements of type int const&.
    assert(contains_a_two(flux::read_only(std::move(seq))));
}
