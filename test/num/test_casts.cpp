
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../test_utils.hpp"

namespace {

// Test unchecked_cast, overflowing_cast and checked_cast between every
// combination of integer types, ensuring that overflow is reported correctly
template <typename From, typename To>
void test_casts()
{
    using namespace flux::num;

    static_assert(flux::num::unchecked_cast<To>(From{0}) == To{0});

    // Cast from zero should never overflow
    {
        constexpr From from{0};

        REQUIRE(unchecked_cast<To>(from) == static_cast<To>(from));

        auto [val, overflowed] = overflowing_cast<To>(from);
        REQUIRE(val == static_cast<To>(from));
        REQUIRE_FALSE(overflowed);

        REQUIRE(checked_cast<To>(from) == static_cast<To>(from));
    }

    // Cast from max should overflow if either:
    // * sizeof(From) is greater than sizeof(To), or
    // * both are the same size, but From is unsigned and To is signed
    {
        constexpr From from = std::numeric_limits<From>::max();

        REQUIRE(unchecked_cast<To>(from) == static_cast<To>(from));

        auto [val, overflowed] = overflowing_cast<To>(from);
        REQUIRE(val == static_cast<To>(from));
        constexpr bool should_overflow =
            (sizeof(From) > sizeof(To)) ||
            ((sizeof(From) == sizeof(To)) && unsigned_integral<From> &&
             signed_integral<To>);
        REQUIRE(overflowed == should_overflow);

        if constexpr (should_overflow) {
            REQUIRE_THROWS_AS(checked_cast<To>(from),
                              flux::unrecoverable_error);
        } else {
            REQUIRE(checked_cast<To>(from) == static_cast<To>(from));
        }
    }

    // If From is signed, cast from min should overflow if To is unsigned
    // or smaller than From
    if constexpr (signed_integral<From>) {
        constexpr From from = std::numeric_limits<From>::lowest();

        REQUIRE(unchecked_cast<To>(from) == static_cast<To>(from));

        auto [val, overflowed] = overflowing_cast<To>(from);

        constexpr bool should_overflow =
            unsigned_integral<To> || (sizeof(To) < sizeof(From));

        REQUIRE(val == static_cast<To>(from));
        REQUIRE(overflowed == should_overflow);

        if constexpr (should_overflow) {
            REQUIRE_THROWS_AS(checked_cast<To>(from),
                              flux::unrecoverable_error);
        } else {
            REQUIRE(checked_cast<To>(from) == static_cast<To>(from));
        }
    }
}

template <typename T0, typename... Types>
void test_casts_helper0()
{
    (test_casts<T0, Types>(), ...);
}

template <typename... Types>
void test_casts_helper1()
{
    (test_casts_helper0<Types, Types...>(), ...);
}

}

TEST_CASE("num.casts")
{
    test_casts_helper1<signed char, unsigned char,
                       signed short, unsigned short,
                       signed int, unsigned int,
                       signed long, unsigned long,
                       signed long long, unsigned long long>();
}