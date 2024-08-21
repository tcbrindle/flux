
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../test_utils.hpp"

#ifdef __clang__
#define COMPILER_IS_CLANG 1
#endif

/*
 * Built-in signed integers
 */

static_assert(flux::num::integral<signed char>);
static_assert(flux::num::integral<short>);
static_assert(flux::num::integral<int>);
static_assert(flux::num::integral<long>);
static_assert(flux::num::integral<long long>);

static_assert(flux::num::signed_integral<signed char>);
static_assert(flux::num::signed_integral<short>);
static_assert(flux::num::signed_integral<int>);
static_assert(flux::num::signed_integral<long>);
static_assert(flux::num::signed_integral<long long>);

static_assert(not flux::num::unsigned_integral<signed char>);
static_assert(not flux::num::unsigned_integral<short>);
static_assert(not flux::num::unsigned_integral<int>);
static_assert(not flux::num::unsigned_integral<long>);
static_assert(not flux::num::unsigned_integral<long long>);

/*
 * Built-in unsigned integers
 */

static_assert(flux::num::integral<unsigned char>);
static_assert(flux::num::integral<unsigned short>);
static_assert(flux::num::integral<unsigned int>);
static_assert(flux::num::integral<unsigned long>);
static_assert(flux::num::integral<unsigned long long>);

static_assert(not flux::num::signed_integral<unsigned char>);
static_assert(not flux::num::signed_integral<unsigned short>);
static_assert(not flux::num::signed_integral<unsigned int>);
static_assert(not flux::num::signed_integral<unsigned long>);
static_assert(not flux::num::signed_integral<unsigned long long>);

static_assert(flux::num::unsigned_integral<unsigned char>);
static_assert(flux::num::unsigned_integral<unsigned short>);
static_assert(flux::num::unsigned_integral<unsigned int>);
static_assert(flux::num::unsigned_integral<unsigned long>);
static_assert(flux::num::unsigned_integral<unsigned long long>);

/*
 * Types which should not be considered integers
 */
static_assert(not flux::num::integral<bool>);
static_assert(not flux::num::integral<char>);
static_assert(not flux::num::integral<wchar_t>);
static_assert(not flux::num::integral<char8_t>);
static_assert(not flux::num::integral<char16_t>);
static_assert(not flux::num::integral<char32_t>);
static_assert(not flux::num::integral<float>);
static_assert(not flux::num::integral<double>);
static_assert(not flux::num::integral<long double>);
static_assert(not flux::num::integral<std::byte>);

/*
 * Compiler-provided extra builtin integer types
 */

// Note that GCC supports __int128, but claims that
// std::is_integral_v<__int128> is false unless -std=gnuXX is passed (but we
// disable extensions in our CMake config)
#if COMPILER_IS_CLANG
static_assert(flux::num::integral<__int128_t>);
static_assert(flux::num::signed_integral<__int128_t>);
static_assert(not flux::num::unsigned_integral<__int128_t>);

static_assert(flux::num::integral<__uint128_t>);
static_assert(not flux::num::signed_integral<__uint128_t>);
static_assert(flux::num::unsigned_integral<__uint128_t>);

// Test a couple of odd-sized custom ints as well
using int7_t = _BitInt(7);
using uint256_t = unsigned _BitInt(256);

static_assert(flux::num::integral<int7_t>);
static_assert(flux::num::signed_integral<int7_t>);
static_assert(not flux::num::unsigned_integral<int7_t>);

static_assert(flux::num::integral<uint256_t>);
static_assert(not flux::num::signed_integral<uint256_t>);
static_assert(flux::num::unsigned_integral<uint256_t>);
#endif
