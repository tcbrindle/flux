
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include "assert.hpp"
#include <array>
#include <string_view>

int main()
{
    std::array arr{1, 2, 3};

    // cycle(seq) returns an infinite sequence. It's common to use this in
    // combination with take() to turn it back into a finite sequence:
    auto cycled1 = flux::take(flux::cycle(arr), 5);
    assert(flux::equal(cycled1, std::array{1, 2, 3, 1, 2}));

    // We can also use a cycled sequence as an argument to zip():
    std::string_view letters = "ABCDE";
    auto zipped = flux::zip(letters, flux::cycle(arr));
    using P = std::pair<char const&, int const&>;
    assert(flux::equal(zipped, std::array{
        P{'A', 1}, P{'B', 2}, P{'C', 3}, P{'D', 1}, P{'E', 2}
    }));

    // Alternatively, we can provide a second argument to cycle(seq, n) to
    // get a finite sequence which repeats the source n times:
    auto cycled2 = flux::cycle(arr, 3);
    assert(flux::equal(cycled2, std::array{1, 2, 3, 1, 2, 3, 1, 2, 3}));
    assert(flux::sum(cycled2) == 18);

    // Note that both versions of cycle() only provide immutable access to their
    // elements. The following would be a compile error:
    // flux::fill(cycled2, 99); // ERROR: cannot assign to const reference
}