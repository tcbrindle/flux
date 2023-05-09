
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux/core/default_impls.hpp>
#include <flux/op/find.hpp>
#include <flux/op/from.hpp>

#include <array>
#include <forward_list>
#include <vector>

namespace {

struct S {
    int i_;
};


using find_fn = decltype(flux::find);

static_assert(std::invocable<find_fn, int[10], float>);

// Incompatible value type
static_assert(not std::invocable<find_fn, int[10], S>);

// Not equality comparable
static_assert(not std::invocable<find_fn, S[10], S>);

template <bool = true>
constexpr bool test_find()
{
    {
        int const ints[] = {0, 1, 2, 3, 4, 5};

        auto cur = flux::find(ints, 3);
        if (cur != 3) {
            return false;
        }

        cur = flux::find(ints, 99);
        if (!flux::is_last(ints, cur)) {
            return false;
        }

        auto lens = flux::from(ints);

        cur = lens.find(3);
        if (cur != 3) {
            return false;
        }

        cur = lens.find(99);
        if (!lens.is_last(cur)) {
            return false;
        }
    }

    return true;
}
static_assert(test_find());

}

TEST_CASE("find")
{
    bool res = test_find<false>();
    REQUIRE(res);

    {
        std::vector<int> vec{1, 2, 3, 4, 5};
        auto idx = flux::find(vec, 3);
        REQUIRE(idx == 2);
    }

    {
        std::vector<int> vec{1, 2, 3, 4, 5};
        auto idx = flux::from(vec).find(99);
        REQUIRE(idx == std::ssize(vec));
    }

}
