
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cassert>
#include <vector>

int main()
{
    std::vector<std::string> const simpsons{
        "Homer", "Marge", "Bart", "Lisa", "Maggie"
    };

    // Bart is a Simpson
    assert(flux::contains(simpsons, "Bart"));

    // Mr Burns is not a Simpson
    assert(not flux::contains(simpsons, "Mr Burns"));
}