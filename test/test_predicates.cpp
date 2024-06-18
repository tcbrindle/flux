
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <algorithm>
#include <array>
#include <string_view>

#include "test_utils.hpp"

namespace {

namespace pred = flux::pred;
using namespace std::string_view_literals;

constexpr auto& all_of = std::ranges::all_of;
constexpr auto& none_of = std::ranges::none_of;

constexpr bool test_predicate_comparators()
{
    std::array const ones{1, 1, 1, 1, 1, 1};
    std::array const twos{2, 2, 2, 2, 2, 2};
    std::array const negatives{-1.0, -2.0, -3.0, -4.0, -5.0};
    std::array const bools{true, true, true, true, true};

    STATIC_CHECK(all_of(ones, pred::true_));
    STATIC_CHECK(none_of(ones, pred::false_));

    STATIC_CHECK(all_of(bools, pred::id));

    STATIC_CHECK(all_of(ones, pred::eq(1)));
    STATIC_CHECK(all_of(ones, pred::neq(22)));
    STATIC_CHECK(all_of(ones, pred::lt(2)));
    STATIC_CHECK(all_of(ones, pred::leq(1)));
    STATIC_CHECK(all_of(ones, pred::gt(0)));
    STATIC_CHECK(all_of(ones, pred::geq(1)));

    STATIC_CHECK(all_of(ones, pred::positive));
    STATIC_CHECK(none_of(ones, pred::negative));
    STATIC_CHECK(all_of(ones, pred::nonzero));

    STATIC_CHECK(all_of(negatives, pred::lt(0)));
    STATIC_CHECK(none_of(negatives, pred::gt(0)));
    STATIC_CHECK(all_of(negatives, pred::leq(-1)));
    STATIC_CHECK(all_of(negatives, pred::geq(-5)));

    STATIC_CHECK(none_of(negatives, pred::positive));
    STATIC_CHECK(all_of(negatives, pred::negative));
    STATIC_CHECK(all_of(negatives, pred::nonzero));

    STATIC_CHECK(all_of(twos, pred::even));
    STATIC_CHECK(none_of(ones, pred::even));
    STATIC_CHECK(all_of(ones, pred::odd));
    STATIC_CHECK(none_of(twos, pred::odd));

    return true;
}
static_assert(test_predicate_comparators());


constexpr bool test_predicate_combiners()
{
    {
        constexpr auto hello_or_world =
            pred::either(pred::eq("hello"sv), pred::eq("world"sv));

        STATIC_CHECK(hello_or_world("hello"sv));
        STATIC_CHECK(hello_or_world("world"sv));
        STATIC_CHECK(!hello_or_world("goodbye"sv));
    }

    {
        constexpr auto hello_or_world =
            pred::eq("hello"sv) || pred::eq("world"sv);

        STATIC_CHECK(hello_or_world("hello"));
        STATIC_CHECK(hello_or_world("world"));
        STATIC_CHECK(!hello_or_world("goodbye"));
    }

    {
        constexpr auto short_ = [](auto s) { return s.size() < 10; };
        constexpr auto shouty = [](auto s) {
            return all_of(s, [](char c) { return c >= 'A' && c <= 'Z'; });
        };

        constexpr auto short_and_shouty = pred::both(short_, shouty);

        STATIC_CHECK(short_and_shouty("HELLO"sv));
        STATIC_CHECK(!short_and_shouty("WHAT A LOVELY DAY WE'RE HAVING"sv));
        STATIC_CHECK(!short_and_shouty("hello?"sv));
    }

    {
        constexpr auto hot = pred::eq("hot");
        constexpr auto cold = pred::eq("cold");
        constexpr auto tepid = pred::neither(hot, cold);

        STATIC_CHECK(tepid("lukewarm"sv));
        STATIC_CHECK(!tepid("hot"sv));
        STATIC_CHECK(!tepid("cold"sv));
    }

    {
        constexpr auto in_names = pred::in("Adam"sv, "Barbara"sv, "Charles"sv);

        STATIC_CHECK(in_names("Adam"));
        STATIC_CHECK(in_names("Barbara"));
        STATIC_CHECK(in_names("Charles"));
        STATIC_CHECK(!in_names("Zacharia"));
    }

    return true;
}
static_assert(test_predicate_combiners());

// Not really predicates, but we'll test them here anyway
constexpr bool test_comparisons()
{
    namespace cmp = flux::cmp;

    struct Test {
        int i;
        double d;

        bool operator==(Test const&) const = default;
    };

    // min of two same-type non-const lvalue references is an lvalue
    {
        int i = 0, j = 1;
        cmp::min(i, j) = 99;
        STATIC_CHECK(i == 99);
        STATIC_CHECK(j == 1);
    }

    // min of same-type mixed-const lvalue refs is a const ref
    {
        int i = 1;
        int const j = 0;
        auto& m = cmp::min(i, j);
        static_assert(std::same_as<decltype(m), int const&>);
        STATIC_CHECK(m == 0);
    }

    // min of same-type lvalue and prvalue is a prvalue
    {
        int const i = 1;
        using M = decltype(cmp::min(i, i + 1));
        static_assert(std::same_as<M, int>);
        STATIC_CHECK(cmp::min(i, i + 1) == 1);
    }

    // Custom comparators work okay with min()
    {
        Test t1{3, 1.0};
        Test t2{2, 1.0};

        auto cmp_test = [](Test t1, Test t2) { return t1.i <=> t2.i; };

        STATIC_CHECK(cmp::min(t1, t2, cmp_test) == t2);
    }

    // If arguments are equal, min() returns the first
    {
        int i = 1, j = 1;
        int& m = cmp::min(i, j);
        STATIC_CHECK(&m == &i);

        Test t1{1, 3.0};
        Test t2{1, 2.0};

        STATIC_CHECK(cmp::min(t1, t2, flux::proj(cmp::compare, &Test::i)) == t1);
    }

    // max of two same-type non-const lvalue references is an lvalue
    {
        int i = 0, j = 1;
        cmp::max(i, j) = 99;
        STATIC_CHECK(i == 0);
        STATIC_CHECK(j == 99);
    }

    // max of same-type mixed-const lvalue refs is a const ref
    {
        int i = 1;
        int const j = 0;
        auto& m = cmp::max(i, j);
        static_assert(std::same_as<decltype(m), int const&>);
        STATIC_CHECK(m == 1);
    }

    // max of same-type lvalue and prvalue is a prvalue
    {
        int const i = 1;
        using M = decltype(cmp::max(i, i + 1));
        static_assert(std::same_as<M, int>);
        STATIC_CHECK(cmp::max(i, i + 1) == 2);
    }

    // Custom comparators work okay with max()
    {
        Test t1{1, 3.0};
        Test t2{1, 2.0};

        auto cmp_test = [](Test t1, Test t2) { return t1.i <=> t2.i; };

        STATIC_CHECK(cmp::max(t1, t2, cmp_test) == t2);
    }

    // If arguments are equal, max() returns the second
    {
        int i = 1, j = 1;
        int& m = cmp::max(i, j);
        STATIC_CHECK(&m == &j);

        Test t1{1, 3.0};
        Test t2{1, 2.0};

        STATIC_CHECK(cmp::max(t1, t2, flux::proj(cmp::compare, &Test::i)) == t2);
    }

    // Reverse comparisons give the expected answer
    {
        int i = 1, j = 2;
        int& min = cmp::min(i, j, cmp::reverse_compare);
        int& max = cmp::max(i, j, cmp::reverse_compare);
        STATIC_CHECK(&min == &j);
        STATIC_CHECK(&max == &i);

        Test t1{1, 3.0};
        Test t2{1, 2.0};

        STATIC_CHECK(&cmp::min(t1, t2, flux::proj(cmp::reverse_compare, &Test::i)) == &t1);
        STATIC_CHECK(&cmp::max(t1, t2, flux::proj(cmp::reverse_compare, &Test::i)) == &t2);
    }

    return true;
}
static_assert(test_comparisons());

struct Test {
    bool operator==(Test const&) const = default;
    constexpr auto operator<=>(Test const&) const {
        return std::partial_ordering::unordered;
    }
};

constexpr bool test_partial_min_max()
{
    namespace cmp = flux::cmp;

    // partial_min works just like min for sensible types
    {
        int i = 100, j = 10;
        int& r = cmp::partial_min(i, j);

        STATIC_CHECK(&r == &j);
    }

    // for partially ordered types, partial_min returns the first element
    // if the arguments are unordered
    {
        Test const t1, t2;

        Test const& r = cmp::partial_min(t1, t2);

        STATIC_CHECK(&r == &t1);
    }

    // partial_max works just like min for sensible types
    {
        int i = 100, j = 10;
        int& r = cmp::partial_max(i, j);

        STATIC_CHECK(&r == &i);
    }

    // for partially ordered types, partial_max returns the second element
    // if the arguments are unordered
    {
        Test const t1, t2;

        Test const& r = cmp::partial_max(t1, t2);

        STATIC_CHECK(&r == &t2);
    }

    return true;
}
static_assert(test_partial_min_max());

}

TEST_CASE("predicates")
{
    REQUIRE(test_predicate_comparators());
    REQUIRE(test_predicate_combiners());
}

TEST_CASE("comparators")
{
    REQUIRE(test_comparisons());
    REQUIRE(test_partial_min_max());
}

