
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux/core/utils.hpp>

#include <climits>
#include <limits>

namespace {

template <typename T>
void test_add()
{
    using flux::detail::int_add;

    constexpr T zero = 0;
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    REQUIRE(int_add(zero, zero) == zero);
    REQUIRE(int_add(T{1}, T{-1}) == zero);

    REQUIRE(int_add(min, zero) == min);
    REQUIRE(int_add(zero, min) == min);
    REQUIRE(int_add(max, zero) == max);
    REQUIRE(int_add(zero, max) == max);

    REQUIRE(int_add(min, max) == T{-1});
    REQUIRE(int_add(max, min) == T{-1});

    REQUIRE_THROWS_AS(int_add(max, T{1}), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(int_add(T{1}, max), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(int_add(min, T{-1}), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(int_add(T{-1}, min), flux::unrecoverable_error);
}

template <typename T>
void test_sub()
{
    using flux::detail::int_sub;

    constexpr T zero = 0;
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    REQUIRE(int_sub(zero, zero) == zero);
    REQUIRE(int_sub(T{1}, T{1}) == zero);
    REQUIRE(int_sub(max, max) == zero);
    REQUIRE(int_sub(min, min) == zero);

    REQUIRE(int_sub(min, zero) == min);
    REQUIRE(int_sub(max, zero) == max);
    REQUIRE(int_sub(zero, max) == T{min + 1});
    REQUIRE(int_sub(T{-1}, max) == min);

    REQUIRE_THROWS_AS(int_sub(min, T{1}), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(int_sub(max, T{-1}), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(int_sub(T{-2}, max), flux::unrecoverable_error);
}

template <typename T>
void test_mul()
{
    using flux::detail::int_mul;

    constexpr T zero = 0;
    constexpr T one = 1;
    constexpr T min = std::numeric_limits<T>::lowest();
    constexpr T max = std::numeric_limits<T>::max();

    REQUIRE(int_mul(zero, zero) == zero);
    REQUIRE(int_mul(zero, one) == zero);
    REQUIRE(int_mul(zero, min) == zero);
    REQUIRE(int_mul(zero, max) == zero);
    REQUIRE(int_mul(one, zero) == zero);
    REQUIRE(int_mul(min, zero) == zero);
    REQUIRE(int_mul(max, zero) == zero);

    REQUIRE(one * one == one);
    REQUIRE(int_mul(one, one) == one);
    REQUIRE(int_mul(one, min) == min);
    REQUIRE(int_mul(one, max) == max);
    REQUIRE(int_mul(min, one) == min);
    REQUIRE(int_mul(max, one) == max);

    REQUIRE(int_mul(max, T{-1}) == T{min + 1});
    REQUIRE(int_mul(T{-1}, max) == T{min + 1});

    REQUIRE_THROWS_AS(int_mul(min, T{-1}), flux::unrecoverable_error);

    // FIXME: Raises sigfpe when not using built-in overflow checking
#if FLUX_HAVE_BUILTIN_OVERFLOW_OPS
    REQUIRE_THROWS_AS(int_mul(T{-1}, min), flux::unrecoverable_error);
#endif

    REQUIRE_THROWS_AS(int_mul(max, T{2}), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(int_mul(T{2}, max), flux::unrecoverable_error);
    REQUIRE_THROWS_AS(int_mul(max, max), flux::unrecoverable_error);


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