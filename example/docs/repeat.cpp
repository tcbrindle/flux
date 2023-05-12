
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <array>
#include <cassert>
#include <string_view>

using namespace std::string_view_literals;

int main()
{
    // flux::repeat(val) is a random-access sequence which endlessly repeats
    // the given value
    auto seq = flux::repeat(3);

    auto cursor = flux::first(seq);
    assert(flux::read_at(seq, cursor) == 3);
    // fast-forward the cursor a lot...
    cursor = flux::next(seq, cursor, 1'000'000);
    assert(flux::read_at(seq, cursor) == 3); // still returning 3!

    // We could use the take adaptor to make a repeat sequence finite...
    auto taken = flux::take(seq, 5);
    assert(flux::equal(taken, std::array{3, 3, 3, 3, 3}));

    // ...but it's easier to use repeat(val, count) instead
    auto police = flux::repeat("hello"sv, 3);
    assert(flux::equal(police, std::array{"hello", "hello", "hello"}));
}