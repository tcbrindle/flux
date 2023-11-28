
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_CONFIG_HPP_INCLUDED
#define FLUX_CORE_CONFIG_HPP_INCLUDED

#include <flux/core/macros.hpp>

#include <concepts>
#include <cstddef>
#include <type_traits>

#define FLUX_ERROR_POLICY_TERMINATE 1
#define FLUX_ERROR_POLICY_UNWIND     2

#define FLUX_OVERFLOW_POLICY_ERROR   10
#define FLUX_OVERFLOW_POLICY_WRAP    11
#define FLUX_OVERFLOW_POLICY_IGNORE  12

#define FLUX_DIVIDE_BY_ZERO_POLICY_ERROR   100
#define FLUX_DIVIDE_BY_ZERO_POLICY_IGNORE  101

// Default error policy is terminate
#define FLUX_DEFAULT_ERROR_POLICY FLUX_ERROR_POLICY_TERMINATE

// Default overflow policy is error in debug builds, wrap in release builds
#ifdef NDEBUG
#  define FLUX_DEFAULT_OVERFLOW_POLICY FLUX_OVERFLOW_POLICY_WRAP
#else
#  define FLUX_DEFAULT_OVERFLOW_POLICY FLUX_OVERFLOW_POLICY_ERROR
#endif // NDEBUG

// Default divide by zero policy is error in debug builds, ignore in release builds
#ifdef NDEBUG
#  define FLUX_DEFAULT_DIVIDE_BY_ZERO_POLICY FLUX_DIVIDE_BY_ZERO_POLICY_IGNORE
#else
#  define FLUX_DEFAULT_DIVIDE_BY_ZERO_POLICY FLUX_DIVIDE_BY_ZERO_POLICY_ERROR
#endif // NDEBUG

// Select which error policy to use
#if defined(FLUX_TERMINATE_ON_ERROR)
#  define FLUX_ERROR_POLICY FLUX_ERROR_POLICY_TERMINATE
#elif defined(FLUX_UNWIND_ON_ERROR)
#  define FLUX_ERROR_POLICY FLUX_ERROR_POLICY_UNWIND
#else
#  define FLUX_ERROR_POLICY FLUX_DEFAULT_ERROR_POLICY
#endif // FLUX_TERMINATE_ON_ERROR

// Should we print an error message before terminating?
#ifndef FLUX_PRINT_ERROR_ON_TERMINATE
#  define FLUX_PRINT_ERROR_ON_TERMINATE 1
#endif // FLUX_PRINT_ERROR_ON_TERMINATE

// Should we test debug assertions?
#ifndef FLUX_ENABLE_DEBUG_ASSERTS
#  ifdef NDEBUG
#    define FLUX_ENABLE_DEBUG_ASSERTS 0
#  else
#    define FLUX_ENABLE_DEBUG_ASSERTS 1
#  endif
#endif

// Select which overflow policy to use
#if defined(FLUX_ERROR_ON_OVERFLOW)
#  define FLUX_OVERFLOW_POLICY FLUX_OVERFLOW_POLICY_ERROR
#elif defined(FLUX_WRAP_ON_OVERFLOW)
#  define FLUX_OVERFLOW_POLICY FLUX_OVERFLOW_POLICY_WRAP
#elif defined(FLUX_IGNORE_OVERFLOW)
#  define FLUX_OVERFLOW_POLICY FLUX_OVERFLOW_POLICY_IGNORE
#else
#  define FLUX_OVERFLOW_POLICY FLUX_DEFAULT_OVERFLOW_POLICY
#endif // FLUX_ERROR_ON_OVERFLOW

// Select which overflow policy to use
#if defined(FLUX_ERROR_ON_DIVIDE_BY_ZERO)
#  define FLUX_DIVIDE_BY_ZERO_POLICY FLUX_DIVIDE_BY_ZERO_POLICY_ERROR
#elif defined(FLUX_IGNORE_DIVIDE_BY_ZERO)
#  define FLUX_DIVIDE_BY_ZERO_POLICY FLUX_DIVIDE_BY_ZERO_POLICY_IGNORE
#else
#  define FLUX_DIVIDE_BY_ZERO_POLICY FLUX_DEFAULT_DIVIDE_BY_ZERO_POLICY
#endif // FLUX_ERROR_ON_DIVIDE_BY_ZERO

// Default int_t is ptrdiff_t
#define FLUX_DEFAULT_INT_TYPE std::ptrdiff_t

// Select which int type to use
#ifndef FLUX_INT_TYPE
#define FLUX_INT_TYPE FLUX_DEFAULT_INT_TYPE
#endif

namespace flux {

FLUX_EXPORT
enum class error_policy {
    terminate = FLUX_ERROR_POLICY_TERMINATE,
    unwind = FLUX_ERROR_POLICY_UNWIND
};

FLUX_EXPORT
enum class overflow_policy {
    ignore = FLUX_OVERFLOW_POLICY_IGNORE,
    wrap = FLUX_OVERFLOW_POLICY_WRAP,
    error = FLUX_OVERFLOW_POLICY_ERROR
};

FLUX_EXPORT
enum class divide_by_zero_policy {
    ignore = FLUX_DIVIDE_BY_ZERO_POLICY_IGNORE,
    error = FLUX_DIVIDE_BY_ZERO_POLICY_ERROR
};

namespace config {

FLUX_EXPORT
using int_type = FLUX_INT_TYPE;
static_assert(std::signed_integral<int_type> && (sizeof(int_type) >= sizeof(std::ptrdiff_t)),
              "Custom FLUX_INT_TYPE must be a signed integer type at least as large as ptrdiff_t");

FLUX_EXPORT
inline constexpr error_policy on_error = static_cast<error_policy>(FLUX_ERROR_POLICY);

FLUX_EXPORT
inline constexpr overflow_policy on_overflow = static_cast<overflow_policy>(FLUX_OVERFLOW_POLICY);

FLUX_EXPORT
inline constexpr divide_by_zero_policy on_divide_by_zero = static_cast<divide_by_zero_policy>(FLUX_DIVIDE_BY_ZERO_POLICY);

FLUX_EXPORT
inline constexpr bool print_error_on_terminate = FLUX_PRINT_ERROR_ON_TERMINATE;

FLUX_EXPORT
inline constexpr bool enable_debug_asserts = FLUX_ENABLE_DEBUG_ASSERTS;

} // namespace config

} // namespace flux

#endif
