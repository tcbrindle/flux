
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <algorithm>

#ifdef USE_MODULES
import flux;
#else
#include <flux/op/all_any_none.hpp>
#endif

namespace {

constexpr auto gt_zero = [](auto i) { return i > 0; };

template <typename T>
constexpr bool test_all(std::initializer_list<T> ilist)
{
    return flux::all(ilist, gt_zero) == std::all_of(ilist.begin(), ilist.end(), gt_zero);
}
static_assert(test_all<int>({}));
static_assert(test_all({1, 2, 3, 4, 5}));
static_assert(test_all({1.0, 2.0, -3.0, 4.0}));

}

TEST_CASE("all with vector")
{
    std::vector<int> vec{1, 2, 3, 4, 5};

    static_assert(flux::contiguous_sequence<decltype(vec)>);

    bool req = flux::all(vec, gt_zero);

    REQUIRE(req);
}
