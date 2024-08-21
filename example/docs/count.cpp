
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include "assert.hpp"
#include <sstream>
#include <vector>

int main()
{
    std::vector<int> vec{1, 2, 3};

    // std::vector is a sized_sequence, so this call is
    // equivalent to `flux::size(vec)` (and so constant time)
    assert(flux::count(vec) == 3);

    // A filtered sequence is never sized, so this call will
    // iterate over each element
    auto filtered = flux::filter(std::move(vec), flux::pred::odd);
    assert(flux::count(filtered) == 2);

    // An istream sequence is single-pass and not sized, so counting
    // the elements will "use up" the sequence
    std::istringstream iss("1 2 3");
    auto seq = flux::from_istream<int>(iss);
    assert(flux::count(seq) == 3);
    assert(flux::is_last(seq, flux::first(seq))); // No more elements!
}