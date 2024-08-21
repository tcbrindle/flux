
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../test_utils.hpp"

namespace {

template <typename T>
constexpr bool test_overflowing_add()
{
    constexpr auto& add = flux::num::overflowing_add;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // adding zero to anything doesn't change it, and doesn't overflow
    auto r = add(zero, zero);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == false);

    r = add(min, zero);
    STATIC_CHECK(r.value == min);
    STATIC_CHECK(r.overflowed == false);

    r = add(zero, min);
    STATIC_CHECK(r.value == min);
    STATIC_CHECK(r.overflowed == false);

    r = add(max, zero);
    STATIC_CHECK(r.value == max);
    STATIC_CHECK(r.overflowed == false);

    r = add(zero, max);
    STATIC_CHECK(r.value == max);
    STATIC_CHECK(r.overflowed == false);

    // Adding one to max wraps and gives min
    r = add(max, one);
    STATIC_CHECK(r.value == min);
    STATIC_CHECK(r.overflowed == true);

    r = add(one, max);
    STATIC_CHECK(r.value == min);
    STATIC_CHECK(r.overflowed == true);

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};

        // "adding" minus one to min gives max
        r = add(min, minus_one);
        STATIC_CHECK(r.value == max);
        STATIC_CHECK(r.overflowed == true);

        r = add(minus_one, min);
        STATIC_CHECK(r.value == max);
        STATIC_CHECK(r.overflowed == true);

        // adding min and max gives -1
        r = add(min, max);
        STATIC_CHECK(r.value == minus_one);
        STATIC_CHECK(r.overflowed == false);

        r = add(max, min);
        STATIC_CHECK(r.value == minus_one);
        STATIC_CHECK(r.overflowed == false);
    }

    return true;
}

static_assert(test_overflowing_add<signed char>());
static_assert(test_overflowing_add<unsigned char>());
static_assert(test_overflowing_add<signed short>());
static_assert(test_overflowing_add<unsigned short>());
static_assert(test_overflowing_add<signed int>());
static_assert(test_overflowing_add<unsigned int>());
static_assert(test_overflowing_add<signed long>());
static_assert(test_overflowing_add<unsigned long>());
static_assert(test_overflowing_add<signed long long>());
static_assert(test_overflowing_add<unsigned long long>());

template <typename T>
constexpr bool test_overflowing_sub()
{
    constexpr auto& sub = flux::num::overflowing_sub;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // Anything minus zero is itself, and doesn't overflow
    auto r = sub(zero, zero);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == false);

    r = sub(one, zero);
    STATIC_CHECK(r.value == one);
    STATIC_CHECK(r.overflowed == false);

    r = sub(min, zero);
    STATIC_CHECK(r.value == min);
    STATIC_CHECK(r.overflowed == false);

    // Anything minus itself is zero, and doesn't overflow
    r = sub(one, one);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == false);

    r = sub(max, max);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == false);

    r = sub(min, min);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == false);

    // min minus a positive value overflows
    r = sub(min, one);
    STATIC_CHECK(r.value == max);
    STATIC_CHECK(r.overflowed == true);

    r = sub(min, max);
    STATIC_CHECK(r.value == one); // weird, but correct
    STATIC_CHECK(r.overflowed == true);

    // max minus a positive value does not overflow
    r = sub(max, one);
    STATIC_CHECK(r.overflowed == false);

    // max minus min differs for signed and unsigned
    r = sub(max, min);
    if constexpr (flux::num::signed_integral<T>) {
        STATIC_CHECK(r.value == T{-1});
        STATIC_CHECK(r.overflowed == true);
    } else {
        STATIC_CHECK(r.value == max);
        STATIC_CHECK(r.overflowed == false);
    }

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};

        r = sub(minus_one, minus_one);
        STATIC_CHECK(r.value == zero);
        STATIC_CHECK(r.overflowed == false);

        r = sub(minus_one, min);
        STATIC_CHECK(r.value == max);
        STATIC_CHECK(r.overflowed == false);

        r = sub(minus_one, max);
        STATIC_CHECK(r.value == min);
        STATIC_CHECK(r.overflowed == false);

        r = sub(min, minus_one);
        STATIC_CHECK(r.overflowed == false);

        r = sub(max, minus_one);
        STATIC_CHECK(r.value == min);
        STATIC_CHECK(r.overflowed == true);
    }

    return true;
}

static_assert(test_overflowing_sub<signed char>());
static_assert(test_overflowing_sub<unsigned char>());
static_assert(test_overflowing_sub<signed short>());
static_assert(test_overflowing_sub<unsigned short>());
static_assert(test_overflowing_sub<signed int>());
static_assert(test_overflowing_sub<unsigned int>());
static_assert(test_overflowing_sub<signed long>());
static_assert(test_overflowing_sub<unsigned long>());
static_assert(test_overflowing_sub<signed long long>());
static_assert(test_overflowing_sub<unsigned long long>());

template <typename T>
constexpr bool test_overflowing_mul()
{
    constexpr auto& mul = flux::num::overflowing_mul;

    constexpr T zero = T{0};
    constexpr T one = T{1};
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    // Anything times zero is zero, and doesn't overflow
    auto r = mul(zero, zero);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == false);

    r = mul(zero, one);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == false);

    r = mul(one, zero);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == false);

    r = mul(min, zero);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == false);

    r = mul(zero, min);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == false);

    r = mul(max, zero);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == false);

    r = mul(max, zero);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == false);

    // Anything times one is itself, and doesn't overflow
    r = mul(one, one);
    STATIC_CHECK(r.value == one);
    STATIC_CHECK(r.overflowed == false);

    r = mul(min, one);
    STATIC_CHECK(r.value == min);
    STATIC_CHECK(r.overflowed == false);

    r = mul(one, min);
    STATIC_CHECK(r.value == min);
    STATIC_CHECK(r.overflowed == false);

    r = mul(max, one);
    STATIC_CHECK(r.value == max);
    STATIC_CHECK(r.overflowed == false);

    r = mul(one, max);
    STATIC_CHECK(r.value == max);
    STATIC_CHECK(r.overflowed == false);

    // max squared is 1 (weirdly), and overflows
    r = mul(max, max);
    STATIC_CHECK(r.value == one);
    STATIC_CHECK(r.overflowed == true);

    // min squared is zero (weirdly), and overflows for signed ints
    r = mul(min, min);
    STATIC_CHECK(r.value == zero);
    STATIC_CHECK(r.overflowed == flux::num::signed_integral<T>);

    // min times max is min (weirdly), and overflows if signed
    r = mul(min, max);
    STATIC_CHECK(r.value == min);
    STATIC_CHECK(r.overflowed == flux::num::signed_integral<T>);

    r = mul(max, min);
    STATIC_CHECK(r.value == min);
    STATIC_CHECK(r.overflowed == flux::num::signed_integral<T>);

    if constexpr (flux::num::signed_integral<T>) {
        constexpr T minus_one = T{-1};

        r = mul(minus_one, one);
        STATIC_CHECK(r.value == minus_one);
        STATIC_CHECK(r.overflowed == false);

        r = mul(one, minus_one);
        STATIC_CHECK(r.value == minus_one);
        STATIC_CHECK(r.overflowed == false);

        r = mul(minus_one, minus_one);
        STATIC_CHECK(r.value == one);
        STATIC_CHECK(r.overflowed == false);

        r = mul(max, minus_one);
        STATIC_CHECK(r.value == min + one);
        STATIC_CHECK(r.overflowed == false);

        r = mul(minus_one, max);
        STATIC_CHECK(r.value == min + one);
        STATIC_CHECK(r.overflowed == false);

        // min * -1 == min, weirdly
        r = mul(min, minus_one);
        STATIC_CHECK(r.value == min);
        STATIC_CHECK(r.overflowed == true);

        r = mul(minus_one, min);
        STATIC_CHECK(r.value == min);
        STATIC_CHECK(r.overflowed == true);
    }

    return true;
}

static_assert(test_overflowing_mul<signed char>());
static_assert(test_overflowing_mul<unsigned char>());
static_assert(test_overflowing_mul<signed short>());
static_assert(test_overflowing_mul<unsigned short>());
static_assert(test_overflowing_mul<signed int>());
static_assert(test_overflowing_mul<unsigned int>());
static_assert(test_overflowing_mul<signed long>());
static_assert(test_overflowing_mul<unsigned long>());
static_assert(test_overflowing_mul<signed long long>());
static_assert(test_overflowing_mul<unsigned long long>());

}

TEST_CASE("num.overflowing_add")
{
    test_overflowing_add<signed char>();
    test_overflowing_add<unsigned char>();
    test_overflowing_add<signed short>();
    test_overflowing_add<unsigned short>();
    test_overflowing_add<signed int>();
    test_overflowing_add<unsigned int>();
    test_overflowing_add<signed long>();
    test_overflowing_add<unsigned long>();
    test_overflowing_add<signed long long>();
    test_overflowing_add<unsigned long long>();
}

TEST_CASE("num.overflowing_sub")
{
    test_overflowing_sub<signed char>();
    test_overflowing_sub<unsigned char>();
    test_overflowing_sub<signed short>();
    test_overflowing_sub<unsigned short>();
    test_overflowing_sub<signed int>();
    test_overflowing_sub<unsigned int>();
    test_overflowing_sub<signed long>();
    test_overflowing_sub<unsigned long>();
    test_overflowing_sub<signed long long>();
    test_overflowing_sub<unsigned long long>();
}

TEST_CASE("num.overflowing_mul")
{
    test_overflowing_mul<signed char>();
    test_overflowing_mul<unsigned char>();
    test_overflowing_mul<signed short>();
    test_overflowing_mul<unsigned short>();
    test_overflowing_mul<signed int>();
    test_overflowing_mul<unsigned int>();
    test_overflowing_mul<signed long>();
    test_overflowing_mul<unsigned long>();
    test_overflowing_mul<signed long long>();
    test_overflowing_mul<unsigned long long>();
}

