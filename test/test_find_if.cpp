
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <doctest/doctest.h>

#include <array>
#include <concepts>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

#ifdef USE_MODULES
import flux;
#else
#    include <flux/core/default_impls.hpp>
#    include <flux/algorithm/find.hpp>
#endif

namespace {

struct S {
    int i_;
};

using find_if_fn = decltype(flux::find_if);

using int_comp = auto(int x) -> bool;
static_assert(std::invocable<find_if_fn, int[10], int_comp>);

using const_int_comp = auto(const int x) -> bool;
static_assert(std::invocable<find_if_fn, int[10], const_int_comp>);

using const_int_ref_comp = auto(const int& x) -> bool;
static_assert(std::invocable<find_if_fn, int[10], const_int_ref_comp>);

// Potential modification during find not allowed
using int_ref_comp = auto(int& x) -> bool;
// static_assert(not std::invocable<find_if_fn, int[10], int_ref_comp>);

// Incompatible predicate type
using S_comp = auto(S x) -> bool;
static_assert(not std::invocable<find_if_fn, int[10], S_comp>);

// Not a predicate
using not_a_predicate = auto(int x) -> S;
static_assert(not std::invocable<find_if_fn, int[10], not_a_predicate>);

constexpr bool test_find_if()
{
    {
        int const ints[] = {0, 1, 2, 3, 4, 5};

        auto is_three = [](int x) { return x == 3; };
        auto is_ten = [](int x) { return x == 10; };
        auto is_negative = [](int x) { return x < 0; };
        auto is_greater_than_4 = [](int x) { return x > 4; };

        auto cur = flux::find_if(ints, is_three);
        if (cur != 3) {
            return false;
        }

        cur = flux::find_if(ints, is_ten);
        if (!flux::is_last(ints, cur)) {
            return false;
        }

        cur = flux::find_if(ints, is_negative);
        if (!flux::is_last(ints, cur)) {
            return false;
        }

        cur = flux::find_if(ints, is_greater_than_4);
        if (cur != 5) {
            return false;
        }

        auto lens = flux::ref(ints);

        cur = lens.find_if(is_three);
        if (cur != 3) {
            return false;
        }

        cur = lens.find_if(is_ten);
        if (!lens.is_last(cur)) {
            return false;
        }

        cur = lens.find_if(is_negative);
        if (!flux::is_last(ints, cur)) {
            return false;
        }

        cur = lens.find_if(is_greater_than_4);
        if (cur != 5) {
            return false;
        }
    }

    return true;
}
static_assert(test_find_if());

} // namespace

TEST_CASE("find_if")
{
    {
        std::vector<int> vec {1, 2, 3, 4, 5};
        auto is_greater_than_3 = [](int x) { return x > 3; };
        auto idx = flux::find_if(vec, is_greater_than_3);
        REQUIRE(idx == 3);
    }

    {
        std::vector<int> vec {1, 2, 3, 4, 5};
        auto is_negative = [](int x) { return x < 0; };
        auto idx = flux::ref(vec).find_if(is_negative);
        REQUIRE(idx == std::ssize(vec));
    }

    {
        std::string_view str = "";
        auto is_lower = [](char x) { return std::islower(static_cast<unsigned char>(x)); };
        auto idx = flux::find_if(str, is_lower);
        REQUIRE(idx == flux::last(str));
    }

    {
        std::string_view str = "abcdefg";
        auto is_upper = [](char x) { return std::isupper(static_cast<unsigned char>(x)); };
        auto idx = flux::find_if(str, is_upper);
        REQUIRE(idx == flux::last(str));
    }

    {
        std::string str = "abcdefg123xyz";
        auto is_numeric = [](char x) { return std::isdigit(static_cast<unsigned char>(x)); };
        auto idx = flux::find_if(str, is_numeric);
        REQUIRE(idx == 7);
    }
}
