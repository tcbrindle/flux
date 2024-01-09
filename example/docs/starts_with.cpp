
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include "assert.hpp"
#include <string>
#include <vector>

using namespace std::literals;

int main()
{
    std::string_view greeting = "Hello world";

    // greeting has "Hello" as a prefix
    assert(flux::starts_with(greeting, "Hello"sv));

    std::vector numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // numbers begins with [1, 2, 3]
    assert(flux::starts_with(numbers, std::array{1, 2, 3}));
}