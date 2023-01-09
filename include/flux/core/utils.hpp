
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_UTILS_HPP_INCLUDED
#define FLUX_CORE_UTILS_HPP_INCLUDED

#include <flux/core/config.hpp>

#include <concepts>
#include <cstdio>
#include <exception>
#include <source_location>
#include <stdexcept>
#include <type_traits>

namespace flux {

/*
 * Useful helpers
 */
template <typename From, typename To>
concept decays_to = std::same_as<std::remove_cvref_t<From>, To>;

namespace detail {

struct copy_fn {
    template <typename T>
    [[nodiscard]]
    constexpr auto operator()(T&& arg) const
        noexcept(std::is_nothrow_convertible_v<T, std::decay_t<T>>)
        -> std::decay_t<T>
    {
        return FLUX_FWD(arg);
    }
};

} // namespace detail

inline constexpr auto copy = detail::copy_fn{};

struct unrecoverable_error : std::logic_error {
    explicit unrecoverable_error(char const* msg) : std::logic_error(msg) {}
};

namespace detail {

struct assertion_failure_fn {
    [[noreturn]]
    inline void operator()(char const* msg, std::source_location loc) const
    {
        if constexpr (config::on_error == error_policy::unwind) {
            char buf[1024];
            std::snprintf(buf, 1024, "%s:%u:%u: Assertion failed: %s",
                          loc.file_name(), loc.line(), loc.column(), msg);
            throw unrecoverable_error(buf);
        } else {
            if constexpr (config::print_error_on_terminate) {
                std::fprintf(stderr, "%s:%u:%u: Assertion failed: %s\n",
                             loc.file_name(), loc.line(), loc.column(), msg);
            }
            std::terminate();
        }
    }
};

inline constexpr auto assertion_failure = assertion_failure_fn{};

struct assert_fn {
    constexpr void operator()(bool cond, char const* msg,
                              std::source_location loc = std::source_location::current()) const
    {
        if (cond) [[likely]] {
            return;
        } else [[unlikely]] {
            assertion_failure(msg, std::move(loc));
        }
    }
};

inline constexpr auto assert_ = assert_fn{};

#define FLUX_ASSERT(cond) (::flux::detail::assert_(cond, #cond))

#ifdef NDEBUG
#  define FLUX_DEBUG_ASSERT(cond)
#else
#  define FLUX_DEBUG_ASSERT FLUX_ASSERT
#endif // NDEBUG

struct bounds_check_fn {
    constexpr void operator()(bool cond, const char* msg,
                              std::source_location loc = std::source_location::current()) const
    {
        if (!std::is_constant_evaluated()) {
            assert_(cond, msg, std::move(loc));
        }
    }
};

inline constexpr auto bounds_check = bounds_check_fn{};

} // namespace detail

#define FLUX_BOUNDS_CHECK(cond) (::flux::detail::bounds_check(cond, #cond))

namespace detail {

template <std::integral To>
struct checked_cast_fn {
    template <std::integral From>
    [[nodiscard]]
    constexpr auto operator()(From from) const -> To
    {
        if constexpr (requires { To{from}; }) {
            return To{from}; // not a narrowing conversion
        } else {
            To to = static_cast<To>(from);
            FLUX_DEBUG_ASSERT(static_cast<From>(to) == from);
            if constexpr (std::is_signed_v<From> != std::is_signed_v<To>) {
                FLUX_DEBUG_ASSERT((to < To{}) == (from < From{}));
            }
            return to;
        }
    }
};

} // namespace detail

template <std::integral To>
inline constexpr auto checked_cast = detail::checked_cast_fn<To>{};

} // namespace flux

#endif
