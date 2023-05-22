
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <array>
#include <iostream>
#include <string_view>

using namespace std::string_view_literals;

int main()
{
    std::array const array{"alpha"sv, "bravo"sv, "charlie"sv, "delta"sv, "echo"sv};

    auto long_words = flux::drop(array, 2);

    // We can use the cursors() adaptor to iterate over the cursors of the
    // sequence (in this case integer indices) and use those to read from the
    // original sequence
    for (auto idx : flux::cursors(long_words)) {
        std::cout << idx << ": " << long_words[idx] << '\n';
    }
    // prints
    // 2: charlie
    // 3: delta
    // 4: echo
}