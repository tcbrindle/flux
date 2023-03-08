
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

#ifdef _GLIBCXX_RELEASE
#if _GLIBCXX_RELEASE < 12
#define COMPILER_IS_GCC11
#endif
#endif

int main()
{
    std::vector<int> v1{1, 2, 3, 4, 5};

    std::vector<int> v2{1, 4, 9, 16, 25};

    // Result is std::strong_ordering because ints have a total order,
    // and less because 2 < 4
    assert(flux::compare(v1, v2) == std::strong_ordering::less);

    // v1 compares equal to v1
    assert(std::is_eq(flux::compare(v1, v1)));

    std::vector<int> v3{1, 2, 3, 4, 5};
    std::vector<int> v4{1, 2, 3};

    // All elements compare equal, but v3 has more elements and so
    // is greater than v4
    assert(flux::compare(v3, v4) == std::strong_ordering::greater);

    // Note that sequences containing NaNs are unordered with the default comparator
    std::vector<double> v5{1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> v6{1.0, 2.0, NAN, 4.0, 5.0};
    assert(flux::compare(v5, v6) == std::partial_ordering::unordered);

    // On most systems we can use std::strong_order as a custom comparator
    // to get a total order for IEEE floats
    // (Note that this is not supported with GCC 11)
    #ifndef COMPILER_IS_GCC11
    if constexpr (std::numeric_limits<double>::is_iec559) {
        assert(flux::compare(v5, v6, std::strong_order) ==
                 std::strong_ordering::less);
    }
    #endif
}