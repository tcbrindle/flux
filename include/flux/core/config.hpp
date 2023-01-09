
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_CONFIG_HPP_INCLUDED
#define FLUX_CORE_CONFIG_HPP_INCLUDED

#include <concepts>
#include <cstddef>
#include <type_traits>

#define FLUX_ERROR_POLICY_TERMINATE 1
#define FLUX_ERROR_POLICY_UNWIND     2

#define FLUX_OVERFLOW_POLICY_ERROR   10
#define FLUX_OVERFLOW_POLICY_WRAP    11
#define FLUX_OVERFLOW_POLICY_IGNORE  12

// Default error policy is terminate
#define FLUX_DEFAULT_ERROR_POLICY FLUX_ERROR_POLICY_TERMINATE

// Default overflow policy is error in debug builds, wrap in release builds
#ifdef NDEBUG
#  define FLUX_DEFAULT_OVERFLOW_POLICY FLUX_OVERFLOW_POLICY_WRAP
#else
#  define FLUX_DEFAULT_OVERFLOW_POLICY FLUX_OVERFLOW_POLICY_ERROR
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

// Default int_t is ptrdiff_t
#define FLUX_DEFAULT_INT_TYPE std::ptrdiff_t

// Select which int type to use
#ifndef FLUX_INT_TYPE
#define FLUX_INT_TYPE FLUX_DEFAULT_INT_TYPE
#endif

namespace flux {

enum class error_policy {
    terminate = FLUX_ERROR_POLICY_TERMINATE,
    unwind = FLUX_ERROR_POLICY_UNWIND
};

enum class overflow_policy {
    ignore = FLUX_OVERFLOW_POLICY_IGNORE,
    wrap = FLUX_OVERFLOW_POLICY_WRAP,
    error = FLUX_OVERFLOW_POLICY_ERROR
};

namespace config {

using int_type = FLUX_INT_TYPE;
static_assert(std::signed_integral<int_type> && (sizeof(int_type) >= sizeof(int)),
              "Custom FLUX_INT_TYPE must be a signed integer type at least as large as int");

inline constexpr error_policy on_error = static_cast<error_policy>(FLUX_ERROR_POLICY);

inline constexpr overflow_policy on_overflow = static_cast<overflow_policy>(FLUX_OVERFLOW_POLICY);

inline constexpr bool print_error_on_terminate = FLUX_PRINT_ERROR_ON_TERMINATE;

} // namespace config

} // namespace flux

#endif
