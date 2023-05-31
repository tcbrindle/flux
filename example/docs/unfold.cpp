
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cassert>
#include <string>
#include <vector>

using namespace std::string_view_literals;

int main()
{
    // We can use unfold() with the identity function to do the equivalent of
    // flux::repeat():
    auto repeated = flux::unfold(std::identity{}, "hello"sv).take(3);

    assert(flux::equal(repeated, std::array{"hello"sv, "hello"sv, "hello"sv}));

    // We can combine unfold() with a mutable lambda to do more sophisticated
    // things, like generating the Fibonacci sequence:
    auto fibs = flux::unfold([next = 1u](unsigned cur) mutable {
        return std::exchange(next, cur + next);
    }, 0u);

    assert(flux::equal(std::move(fibs).take(10),
                       std::array{0, 1, 1, 2, 3, 5, 8, 13, 21, 34}));
}