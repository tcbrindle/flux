
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cassert>
#include <string>
#include <vector>

using namespace std::literals;

int main()
{
    std::string_view greeting = "Hello world";

    // greeting has "world" as a suffix
    assert(flux::ends_with(greeting, "world"sv));

    std::vector numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // numbers ends with [8, 9, 10]
    assert(flux::ends_with(numbers, std::array{8, 9, 10}));
}