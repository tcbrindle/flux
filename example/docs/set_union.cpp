
// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <flux.hpp>

#include <array>
#include <cassert>

int main()
{
    std::array arr1{1, 3, 5};
    std::array arr2{1, 2, 4, 6};

    auto merged = flux::set_union(flux::ref(arr1), flux::ref(arr2));

    assert(flux::equal(merged, std::array{1, 2, 3, 4, 5, 6}));
}