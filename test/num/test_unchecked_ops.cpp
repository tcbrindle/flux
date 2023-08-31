
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../catch.hpp"

#include <flux/core/numeric.hpp>

#include <iostream>

namespace {

template <typename T>
void test_unchecked_add()
{
    constexpr auto& add = flux::num::unchecked_add;

    static_assert(std::same_as<std::invoke_result_t<decltype(add), T, T>, T>);

    constexpr T zero = 0;
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    REQUIRE(add(zero, zero) == zero);
    REQUIRE(add(T{1}, T(-1)) == zero);

    REQUIRE(add(min, zero) == min);
    REQUIRE(add(zero, min) == min);
    REQUIRE(add(max, zero) == max);
    REQUIRE(add(zero, max) == max);

    REQUIRE(add(min, max) == T(-1));
    REQUIRE(add(max, min) == T(-1));

    REQUIRE(sizeof(T) <= 16);
}

template <typename T>
void test_unchecked_sub()
{
    constexpr auto& sub = flux::num::unchecked_sub;

    constexpr T zero = 0;
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    REQUIRE(sub(zero, zero) == zero);
    REQUIRE(sub(T{1}, T{1}) == zero);
    REQUIRE(sub(max, max) == zero);
    REQUIRE(sub(min, min) == zero);

    REQUIRE(sub(min, zero) == min);
    REQUIRE(sub(max, zero) == max);
    REQUIRE(sub(zero, max) == T{min + 1});
    REQUIRE(sub(T(-1), max) == min);
}

template <typename T>
void test_unchecked_mul()
{
    constexpr auto& mul = flux::num::unchecked_mul;

    constexpr T zero = 0;
    constexpr T one = 1;
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    REQUIRE(mul(zero, zero) == zero);
    REQUIRE(mul(zero, one) == zero);
    REQUIRE(mul(zero, min) == zero);
    REQUIRE(mul(zero, max) == zero);
    REQUIRE(mul(one, zero) == zero);
    REQUIRE(mul(min, zero) == zero);
    REQUIRE(mul(max, zero) == zero);

    REQUIRE(one * one == one);
    REQUIRE(mul(one, one) == one);
    REQUIRE(mul(one, min) == min);
    REQUIRE(mul(one, max) == max);
    REQUIRE(mul(min, one) == min);
    REQUIRE(mul(max, one) == max);

    if constexpr (flux::num::signed_integral<T>) {
        REQUIRE(mul(max, T{-1}) == T{min + 1});
        REQUIRE(mul(T{-1}, max) == T{min + 1});
    }
}

template <typename T>
void test_unchecked_div()
{
    constexpr auto& div = flux::num::unchecked_div;

    constexpr T zero{0};
    constexpr T one{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    REQUIRE(div(T{10}, T{5}) == T{2});

    // zero divided by anything is zero
    REQUIRE(div(zero, one) == zero);
    REQUIRE(div(zero, max) == zero);

    // one divided by one is one (test this once)
    REQUIRE(div(one, one) == one);

    // anything divided by one is unchanged
    REQUIRE(div(max, one) == max);
    REQUIRE(div(min, one) == min);

    // For signed type, dividing by -1 negates (except for the lowest value)
    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one{-1};
        REQUIRE(div(one, minus_one) == minus_one);
        REQUIRE(div(minus_one, one) == minus_one);
        REQUIRE(div(minus_one, minus_one) == one);
        REQUIRE(div(max, minus_one) == T{min + 1});
    }
}

template <typename T>
void test_unchecked_mod()
{
    constexpr auto& mod = flux::num::unchecked_mod;

    constexpr T zero{0};
    constexpr T one{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // 0 % x == 0 for all x
    REQUIRE(mod(zero, one) == zero);
    REQUIRE(mod(zero, T{2}) == zero);
    REQUIRE(mod(zero, max) == zero);

    // x % 1 == 0 for all x
    REQUIRE(mod(one, one) == zero);
    REQUIRE(mod(T{2}, one) == zero);
    REQUIRE(mod(min, one) == zero);
    REQUIRE(mod(max, one) == zero);

    // x % max == x for all x < max
    REQUIRE(mod(one, max) == one);
    REQUIRE(mod(T{2}, max) == T{2});
    REQUIRE(mod(max, max) == zero);
};

template <typename T, typename U>
struct test_unchecked_shl {
    void operator()() const
    {
        constexpr auto& shl = flux::num::unchecked_shl;

        constexpr auto width = sizeof(T) * CHAR_BIT;
        constexpr T zero = T{0};
        constexpr T min = std::numeric_limits<T>::lowest();
        constexpr T max = std::numeric_limits<T>::max();

        REQUIRE(shl(T{1}, U{0}) == T{1});
        REQUIRE(shl(T{1}, U{1}) == T{2});
        REQUIRE(shl(T{1}, U{2}) == T{4});

        if constexpr (flux::num::signed_integral<T>) {
            REQUIRE(shl(T{1}, U{width - 1}) == min);
            REQUIRE(shl(min, U{1}) == zero);
        } else {
            REQUIRE(shl(T{1}, U{width - 1}) == T{1} + max / T{2});
        }
    }
};

template <typename T, typename U>
struct test_unchecked_shr {
    void operator()() const
    {
        constexpr auto& shr = flux::num::unchecked_shr;

        constexpr auto width = sizeof(T) * CHAR_BIT;
        constexpr T zero = T{0};
        constexpr T min = std::numeric_limits<T>::lowest();
        constexpr T max = std::numeric_limits<T>::max();

        REQUIRE(shr(max, U{1}) == max / T{2});
        REQUIRE(shr(max, U{2}) == max / T{4});
        REQUIRE(shr(max, U{3}) == max / T{8});

        if constexpr (flux::num::unsigned_integral<T>) {
            REQUIRE(shr(max, U{width - 1}) == T{1});
        } else {
            REQUIRE(shr(max, U{width - 1}) == zero);

            REQUIRE(shr(min, U{1}) == min / T{2});
            REQUIRE(shr(min, U{2}) == min / T{4});
            REQUIRE(shr(min, U{3}) == min / T{8});

            REQUIRE(shr(min, U{width - 1}) == T{-1});
        }
    }
};

}

TEST_CASE("num.unchecked_add")
{
    test_unchecked_add<signed char>();
    test_unchecked_add<unsigned char>();
    test_unchecked_add<signed short>();
    test_unchecked_add<unsigned short>();
    test_unchecked_add<signed int>();
    test_unchecked_add<unsigned int>();
    test_unchecked_add<signed long>();
    test_unchecked_add<unsigned long>();
    test_unchecked_add<signed long long>();
    test_unchecked_add<unsigned long long>();
}

TEST_CASE("num.unchecked_sub")
{
    test_unchecked_sub<signed char>();
    test_unchecked_sub<unsigned char>();
    test_unchecked_sub<signed short>();
    test_unchecked_sub<unsigned short>();
    test_unchecked_sub<signed int>();
    test_unchecked_sub<unsigned int>();
    test_unchecked_sub<signed long>();
    test_unchecked_sub<unsigned long>();
    test_unchecked_sub<signed long long>();
    test_unchecked_sub<unsigned long long>();
}

TEST_CASE("num.unchecked_mul")
{
    test_unchecked_mul<signed char>();
    test_unchecked_mul<unsigned char>();
    test_unchecked_mul<signed short>();
    test_unchecked_mul<unsigned short>();
    test_unchecked_mul<signed int>();
    test_unchecked_mul<unsigned int>();
    test_unchecked_mul<signed long>();
    test_unchecked_mul<unsigned long>();
    test_unchecked_mul<signed long long>();
    test_unchecked_mul<unsigned long long>();
}

TEST_CASE("num.unchecked_div")
{
    test_unchecked_div<signed char>();
    test_unchecked_div<unsigned char>();
    test_unchecked_div<signed short>();
    test_unchecked_div<unsigned short>();
    test_unchecked_div<signed int>();
    test_unchecked_div<unsigned int>();
    test_unchecked_div<signed long>();
    test_unchecked_div<unsigned long>();
    test_unchecked_div<signed long long>();
    test_unchecked_div<unsigned long long>();
}

TEST_CASE("num.unchecked_mod")
{
    test_unchecked_mod<signed char>();
    test_unchecked_mod<unsigned char>();
    test_unchecked_mod<signed short>();
    test_unchecked_mod<unsigned short>();
    test_unchecked_mod<signed int>();
    test_unchecked_mod<unsigned int>();
    test_unchecked_mod<signed long>();
    test_unchecked_mod<unsigned long>();
    test_unchecked_mod<signed long long>();
    test_unchecked_mod<unsigned long long>();
}

template <template <typename...> class Fn, typename T0, typename... Types>
void test_helper0()
{
    (Fn<T0, Types>{}(), ...);
}

template <template <typename...> typename Fn, typename... Types>
void test_helper1()
{
    (test_helper0<Fn, Types, Types...>(), ...);
}

TEST_CASE("num.unchecked_shl")
{
    test_helper1<test_unchecked_shl,
                 signed char, unsigned char,
                 signed short, unsigned short,
                 signed int, unsigned int,
                 signed long, unsigned long,
                 signed long long, unsigned long long>();
}

TEST_CASE("num.unchecked_shr")
{
    test_helper1<test_unchecked_shr,
                 signed char, unsigned char,
                 signed short, unsigned short,
                 signed int, unsigned int,
                 signed long, unsigned long,
                 signed long long, unsigned long long>();
}