
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <doctest/doctest.h>

#include <cstdint>
#include <limits>

#ifdef USE_MODULES
import flux;
#else
#include <flux/core/utils.hpp>
#endif

namespace {

template <typename T>
void test_add()
{
    auto add = flux::num::checked_add;

    constexpr T zero = 0;
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    REQUIRE(add(zero, zero) == zero);
    REQUIRE(add(T{1}, T{-1}) == zero);

    REQUIRE(add(min, zero) == min);
    REQUIRE(add(zero, min) == min);
    REQUIRE(add(max, zero) == max);
    REQUIRE(add(zero, max) == max);

    REQUIRE(add(min, max) == T{-1});
    REQUIRE(add(max, min) == T{-1});

    REQUIRE_THROWS_AS(add(max, T{1}), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(add(T{1}, max), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(add(min, T{-1}), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(add(T{-1}, min), flux::unrecoverable_error);
}

template <typename T>
void test_sub()
{
    auto sub = flux::num::checked_sub;

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
    REQUIRE(sub(T{-1}, max) == min);

    REQUIRE_THROWS_AS(sub(zero, min), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(sub(min, T{1}), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(sub(max, T{-1}), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(sub(T{-2}, max), flux::unrecoverable_error);
}

template <typename T>
void test_mul()
{
    auto mul = flux::num::checked_mul;

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

    REQUIRE(mul(max, T{-1}) == T{min + 1});
    REQUIRE(mul(T{-1}, max) == T{min + 1});

    REQUIRE_THROWS_AS(mul(min, T{-1}), flux::unrecoverable_error);

#ifndef USE_MODULES
    // FIXME: Raises sigfpe when not using built-in overflow checking
    if constexpr (flux::num::detail::use_builtin_overflow_ops) {
        REQUIRE_THROWS_AS(mul(T{-1}, min), flux::unrecoverable_error);
    }
#endif

    REQUIRE_THROWS_AS(mul(max, T{2}), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(mul(T{2}, max), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(mul(max, max), flux::unrecoverable_error);


}

template <typename T>
void test()
{
    test_add<T>();
    test_sub<T>();
    test_mul<T>();
}

}

TEST_CASE("integer overflow")
{
    test<int8_t>();
    test<int16_t>();
    test<int32_t>();
    test<int64_t>();
}