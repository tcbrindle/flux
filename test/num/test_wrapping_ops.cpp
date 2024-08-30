
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../test_utils.hpp"

namespace {

template <typename T>
void test_wrapping_add()
{
    constexpr auto& add = flux::num::wrapping_add;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // adding zero to anything doesn't change it
    REQUIRE(add(zero, zero) == zero);
    REQUIRE(add(one, zero) == one);
    REQUIRE(add(zero, one) == one);
    REQUIRE(add(min, zero) == min);
    REQUIRE(add(zero, min) == min);
    REQUIRE(add(max, zero) == max);
    REQUIRE(add(zero, max) == max);

    // Adding one to max wraps and gives min
    REQUIRE(add(max, one) == min);
    REQUIRE(add(one, max) == min);

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};

        // "adding" minus one to min gives max
        REQUIRE(add(min, minus_one) == max);
        REQUIRE(add(minus_one, min) == max);

        // adding min and max gives -1
        REQUIRE(add(min, max) == minus_one);
        REQUIRE(add(max, min) == minus_one);
    }
}

template <typename T>
void test_wrapping_sub()
{
    constexpr auto& sub = flux::num::wrapping_sub;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // Anything minus itself is zero
    REQUIRE(sub(zero, zero) == zero);
    REQUIRE(sub(one, one) == zero);
    REQUIRE(sub(max, max) == zero);
    REQUIRE(sub(min, min) == zero);

    // x - zero is x
    REQUIRE(sub(one, zero) == one);
    REQUIRE(sub(min, zero) == min);
    REQUIRE(sub(max, zero) == max);

    REQUIRE(sub(zero, max) == T{min + 1});

    // min minus one is max
    REQUIRE(sub(min, one) == max);

    // zero minus min is min, weirdly
    REQUIRE(sub(zero, min) == min);

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};
        REQUIRE(sub(minus_one, max) == min);
        REQUIRE(sub(max, minus_one) == min);
    }
}

template <typename T>
void test_wrapping_mul()
{
    constexpr auto& mul = flux::num::wrapping_mul;

    constexpr T zero = 0;
    constexpr T one = 1;
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // anything times zero is zero
    REQUIRE(mul(zero, zero) == zero);
    REQUIRE(mul(zero, one) == zero);
    REQUIRE(mul(zero, min) == zero);
    REQUIRE(mul(zero, max) == zero);
    REQUIRE(mul(one, zero) == zero);
    REQUIRE(mul(min, zero) == zero);
    REQUIRE(mul(max, zero) == zero);

    // anything times one is one
    REQUIRE(mul(one, one) == one);
    REQUIRE(mul(one, min) == min);
    REQUIRE(mul(one, max) == max);
    REQUIRE(mul(min, one) == min);
    REQUIRE(mul(max, one) == max);

    // max times max is one, weirdly
    REQUIRE(mul(max, max) == one);
    // min times max is min
    REQUIRE(mul(min, max) == min);
    REQUIRE(mul(max, min) == min);

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};
        REQUIRE(mul(one, minus_one) == minus_one);
        REQUIRE(mul(minus_one, minus_one) == one);

        REQUIRE(mul(max, minus_one) == T{min + one});
        REQUIRE(mul(minus_one, max) == T{min + one});

        REQUIRE(mul(min, minus_one) == min);
        REQUIRE(mul(minus_one, min) == min);
    }
}

template <typename T>
void test_wrapping_neg()
{
    constexpr auto& neg = flux::num::wrapping_neg;

    constexpr T zero = 0;
    constexpr T one = 1;
    constexpr T minus_one = -1;
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    REQUIRE(neg(zero) == zero);
    REQUIRE(neg(one) == minus_one);
    REQUIRE(neg(minus_one) == one);
    REQUIRE(neg(neg(one)) == one);
    REQUIRE(neg(neg(minus_one)) == minus_one);
    REQUIRE(neg(max) == min + 1);
    REQUIRE(neg(min + 1) == max);
    REQUIRE(neg(min) == min);
    REQUIRE(neg(neg(min)) == min);
}

}

TEST_CASE("num.wrapping_add")
{
    test_wrapping_add<signed char>();
    test_wrapping_add<unsigned char>();
    test_wrapping_add<signed short>();
    test_wrapping_add<unsigned short>();
    test_wrapping_add<signed int>();
    test_wrapping_add<unsigned int>();
    test_wrapping_add<signed long>();
    test_wrapping_add<unsigned long>();
    test_wrapping_add<signed long long>();
    test_wrapping_add<unsigned long long>();
}

TEST_CASE("num.wrapping_sub")
{
    test_wrapping_sub<signed char>();
    test_wrapping_sub<unsigned char>();
    test_wrapping_sub<signed short>();
    test_wrapping_sub<unsigned short>();
    test_wrapping_sub<signed int>();
    test_wrapping_sub<unsigned int>();
    test_wrapping_sub<signed long>();
    test_wrapping_sub<unsigned long>();
    test_wrapping_sub<signed long long>();
    test_wrapping_sub<unsigned long long>();
}

TEST_CASE("num.wrapping_mul")
{
    test_wrapping_mul<signed char>();
    test_wrapping_mul<unsigned char>();
    test_wrapping_mul<signed short>();
    test_wrapping_mul<unsigned short>();
    test_wrapping_mul<signed int>();
    test_wrapping_mul<unsigned int>();
    test_wrapping_mul<signed long>();
    test_wrapping_mul<unsigned long>();
    test_wrapping_mul<signed long long>();
    test_wrapping_mul<unsigned long long>();
}

TEST_CASE("num.wrapping_neg")
{
    test_wrapping_neg<signed char>();
    test_wrapping_neg<signed short>();
    test_wrapping_neg<signed int>();
    test_wrapping_neg<signed long>();
    test_wrapping_neg<signed long long>();
}

