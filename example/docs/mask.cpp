
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
    std::array values{"one"sv, "two"sv, "three"sv, "four"sv, "five"sv};

    std::array selectors{true, false, true, false, true};

    // flux::mask() selects those elements of values for which the corresponding
    // element of selectors is true
    auto masked = flux::mask(values, selectors);
    assert(flux::equal(masked, std::array{"one"sv, "three"sv, "five"sv}));

    // Note that the selectors sequence can have any element type which is
    // explicitly convertible to bool
    std::array int_selectors{0, 0, 0, 1, 1};

    auto masked2 = flux::mask(values, int_selectors);
    assert(flux::equal(masked2, std::array{"four"sv, "five"sv}));
}