
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
            std::snprintf(buf, 1024, "%s:%u: Fatal error: %s",
                          loc.file_name(), loc.line(), msg);
            throw unrecoverable_error(buf);
        } else {
            if constexpr (config::print_error_on_terminate) {
                std::fprintf(stderr, "%s:%u: Fatal error: %s\n",
                             loc.file_name(), loc.line(), msg);
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

#define FLUX_ASSERT(cond) (::flux::detail::assert_(cond, "assertion '" #cond "' failed"))

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

#define FLUX_BOUNDS_CHECK(cond) (::flux::detail::bounds_check(cond, "out of bounds sequence access"))

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

namespace detail {

struct wrapping_add_fn {
    template <std::signed_integral T>
    constexpr auto operator()(T lhs, T rhs) const noexcept -> T
    {
        using U = std::make_unsigned_t<T>;
        return static_cast<T>(static_cast<U>(lhs) + static_cast<U>(rhs));
    }
};

struct wrapping_sub_fn {
    template <std::signed_integral T>
    constexpr auto operator()(T lhs, T rhs) const noexcept -> T
    {
        using U = std::make_unsigned_t<T>;
        return static_cast<T>(static_cast<U>(lhs) - static_cast<U>(rhs));
    }
};

struct wrapping_mul_fn {
    template <std::signed_integral T>
    constexpr auto operator()(T lhs, T rhs) const noexcept -> T
    {
        using U = std::make_unsigned_t<T>;
        return static_cast<T>(static_cast<U>(lhs) * static_cast<U>(rhs));
    }
};

inline constexpr auto wrapping_add = wrapping_add_fn{};
inline constexpr auto wrapping_sub = wrapping_sub_fn{};
inline constexpr auto wrapping_mul = wrapping_mul_fn{};

#if defined(__has_builtin)
#  if __has_builtin(__builtin_add_overflow) && \
      __has_builtin(__builtin_sub_overflow) && \
      __has_builtin(__builtin_mul_overflow)
#    define FLUX_HAVE_BUILTIN_OVERFLOW_OPS 1
#  endif
#endif

struct add_overflow_fn {
    template <std::signed_integral T>
    constexpr auto operator()(T lhs, T rhs, T* result) const noexcept -> bool
    {
#ifdef FLUX_HAVE_BUILTIN_OVERFLOW_OPS
        return __builtin_add_overflow(lhs, rhs, result);
#else
        *result = wrapping_add(lhs, rhs);
        return ((lhs < T{}) == (rhs < T{})) && ((lhs < T{}) != (*result < T{}));
#endif
    }
};

struct sub_overflow_fn {
    template <std::signed_integral T>
    constexpr auto operator()(T lhs, T rhs, T* result) const noexcept -> bool
    {
#ifdef FLUX_HAVE_BUILTIN_OVERFLOW_OPS
        return __builtin_sub_overflow(lhs, rhs, result);
#else
        *result = wrapping_sub(lhs, rhs);
        return (lhs > T{} && rhs < T{} && *result < T{}) ||
               (lhs < T{} && rhs > T{} && *result > T{});
#endif
    }
};

struct mul_overflow_fn {
    template <std::signed_integral T>
    constexpr auto operator()(T lhs, T rhs, T* result) const noexcept -> bool
    {
#ifdef FLUX_HAVE_BUILTIN_OVERFLOW_OPS
        return __builtin_mul_overflow(lhs, rhs, result);
#else
        *result = wrapping_mul(lhs, rhs);
        return lhs != 0 && *result/lhs != rhs;
#endif
    }
};

inline constexpr auto add_overflow = add_overflow_fn{};
inline constexpr auto sub_overflow = sub_overflow_fn{};
inline constexpr auto mul_overflow = mul_overflow_fn{};

struct int_add_fn {
    template <std::signed_integral T>
    constexpr auto operator()(T lhs, T rhs,
                              std::source_location loc = std::source_location::current()) const -> T
    {
        if (std::is_constant_evaluated()) {
            return lhs + rhs;
        } else {
            if constexpr (config::on_overflow == overflow_policy::ignore) {
                return lhs + rhs;
            } else if constexpr (config::on_overflow == overflow_policy::wrap) {
                return wrapping_add(lhs, rhs);
            } else {
                bool overflowed = add_overflow(lhs, rhs, &lhs);
                if (overflowed) {
                    assertion_failure("Signed overflow in addition", loc);
                }
                return lhs;
            }
        }
    }
};

struct int_sub_fn {
    template <std::signed_integral T>
    constexpr auto operator()(T lhs, T rhs,
                              std::source_location loc = std::source_location::current()) const -> T
    {
        if (std::is_constant_evaluated()) {
            return lhs - rhs;
        } else {
            if constexpr (config::on_overflow == overflow_policy::ignore) {
                return lhs - rhs;
            } else if constexpr (config::on_overflow == overflow_policy::wrap) {
                return wrapping_sub(lhs, rhs);
            } else {
                bool overflowed = sub_overflow(lhs, rhs, &lhs);
                if (overflowed) {
                    assertion_failure("Signed overflow in subtraction", loc);
                }
                return lhs;
            }
        }
    }
};

struct int_mul_fn {
    template <std::signed_integral T>
    constexpr auto operator()(T lhs, T rhs,
                              std::source_location loc = std::source_location::current()) const -> T
    {
        if (std::is_constant_evaluated()) {
            return lhs * rhs;
        } else {
            if constexpr (config::on_overflow == overflow_policy::ignore) {
                return lhs * rhs;
            } else if constexpr (config::on_overflow == overflow_policy::wrap) {
                return wrapping_mul(lhs, rhs);
            } else {
                bool overflowed = mul_overflow(lhs, rhs, &lhs);
                if (overflowed) {
                    assertion_failure("Signed overflow in multiplication", loc);
                }
                return lhs;
            }
        }
    }
};

inline constexpr auto int_add = int_add_fn{};
inline constexpr auto int_sub = int_sub_fn{};
inline constexpr auto int_mul = int_mul_fn{};

} // namespace detail

} // namespace flux

#endif
