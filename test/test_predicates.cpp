
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux/core/predicates.hpp>

#include "test_utils.hpp"

#include <array>
#include <string_view>

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

}

TEST_CASE("predicates")
{
    REQUIRE(test_predicate_comparators());
    REQUIRE(test_predicate_combiners());
}

