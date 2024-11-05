
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

using find_if_not_fn = decltype(flux::find_if_not);

using int_comp = auto(int x) -> bool;
static_assert(std::invocable<find_if_not_fn, int[10], int_comp>);

using const_int_comp = auto(const int x) -> bool;
static_assert(std::invocable<find_if_not_fn, int[10], const_int_comp>);

using const_int_ref_comp = auto(const int& x) -> bool;
static_assert(std::invocable<find_if_not_fn, int[10], const_int_ref_comp>);

// Incompatible predicate type
using S_comp = auto(S x) -> bool;
static_assert(not std::invocable<find_if_not_fn, int[10], S_comp>);

// Not a predicate
using not_a_predicate = auto(int x) -> S;
static_assert(not std::invocable<find_if_not_fn, int[10], not_a_predicate>);

constexpr bool test_find_if_not()
{
    {
        int const ints[] = {0, 1, 2, 3, 4, 5};

        auto is_three = [](int x) { return x == 3; };
        auto is_zero = [](int x) { return x == 0; };
        auto is_greater_than_zero = [](int x) { return x >= 0; };
        auto is_0_1_2_or_3 = [](int x) { return 0 <= x && x <= 3; };

        auto cur = flux::find_if_not(ints, is_three);
        if (cur != 0) {
            return false;
        }

        cur = flux::find_if_not(ints, is_zero);
        if (cur != 1) {
            return false;
        }

        cur = flux::find_if_not(ints, is_greater_than_zero);
        if (!flux::is_last(ints, cur)) {
            return false;
        }

        cur = flux::find_if_not(ints, is_0_1_2_or_3);
        if (cur != 4) {
            return false;
        }

        auto lens = flux::ref(ints);

        cur = lens.find_if_not(is_three);
        if (cur != 0) {
            return false;
        }

        cur = lens.find_if_not(is_zero);
        if (cur != 1) {
            return false;
        }

        cur = lens.find_if_not(is_greater_than_zero);
        if (!flux::is_last(ints, cur)) {
            return false;
        }

        cur = lens.find_if_not(is_0_1_2_or_3);
        if (cur != 4) {
            return false;
        }
    }

    return true;
}
static_assert(test_find_if_not());

} // namespace

TEST_CASE("find_if_not")
{
    REQUIRE(test_find_if_not());

    {
        std::vector<int> vec {1, 2, 3, 4, 5};
        auto is_odd = [](int x) { return x % 2 == 1; };
        auto idx = flux::find_if_not(vec, is_odd);
        REQUIRE(idx == 1);
    }

    {
        std::vector<int> vec {1, 2, 3, 4, 5};
        auto is_positive = [](int x) { return x > 0; };
        auto idx = flux::ref(vec).find_if_not(is_positive);
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
        auto is_lower = [](char x) { return std::islower(static_cast<unsigned char>(x)); };
        auto idx = flux::find_if_not(str, is_lower);
        REQUIRE(idx == flux::last(str));
    }

    {
        std::string str = "123abc";
        auto is_numeric = [](char x) { return std::isdigit(static_cast<unsigned char>(x)); };
        auto idx = flux::find_if_not(str, is_numeric);
        REQUIRE(idx == 3);
    }
}
