
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <doctest/doctest.h>

#include <array>
#include <concepts>
#include <string>
#include <string_view>
#include <vector>

#ifdef USE_MODULES
import flux;
#else
#include <flux/core/default_impls.hpp>
#include <flux/algorithm/find.hpp>
#endif

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

        auto lens = flux::ref(ints);

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
    {
        std::vector<int> vec{1, 2, 3, 4, 5};
        auto idx = flux::find(vec, 3);
        REQUIRE(idx == 2);
    }

    {
        std::vector<int> vec{1, 2, 3, 4, 5};
        auto idx = flux::ref(vec).find(99);
        REQUIRE(idx == std::ssize(vec));
    }

    {
        std::string_view str = "abcdefg";
        auto idx = flux::find(str, 'd');
        REQUIRE(idx == 3);
    }

    {
        std::string str = "";
        auto idx = flux::find(str, 'a');
        REQUIRE(idx == flux::last(str));
    }
}
