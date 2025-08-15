
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include "assert.hpp"
#include <array>
#include <string_view>

using namespace std::string_view_literals;

int main()
{
    // flux::repeat(val) is an iterable which endlessly repeats
    // the given value
    auto rep = flux::repeat(3);

    auto ctx = flux::iterate(rep);
    assert(flux::next_element(ctx).value() == 3);
    assert(flux::next_element(ctx).value() == 3);
    assert(flux::next_element(ctx).value() == 3); // still returning 3!

    // We could use the take adaptor to make a repeat sequence finite...
    auto taken = flux::take(rep, 5);
    assert(flux::equal(taken, std::array{3, 3, 3, 3, 3}));

    // ...but it's better to use repeat(val, count) instead, as this is random-access
    auto police = flux::repeat("hello"sv, 3);
    assert(flux::equal(police, std::array{"hello", "hello", "hello"}));
    static_assert(flux::random_access_sequence<decltype(police)>);
}