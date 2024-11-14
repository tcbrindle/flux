
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_NUMERIC_HPP_INCLUDED
#define FLUX_CORE_NUMERIC_HPP_INCLUDED

#include <flux/core/assert.hpp>
#include <flux/core/utils.hpp>
#include <flux/core/detail/jtckdint.h>

#include <climits>
#include <cstdint>
#include <limits>
#include <utility>

namespace flux::num {

FLUX_EXPORT
template <typename T>
concept integral =
    std::integral<T> &&
    !flux::detail::any_of<T, bool, char, wchar_t, char8_t, char16_t, char32_t>;

FLUX_EXPORT
template <typename T>
concept signed_integral = integral<T> && std::signed_integral<T>;

FLUX_EXPORT
template <typename T>
concept unsigned_integral = integral<T> && std::unsigned_integral<T>;

FLUX_EXPORT
template <integral T>
struct overflow_result {
    T value;
    bool overflowed;
};

namespace detail {

template <integral To>
struct unchecked_cast_fn {
    template <integral From>
    [[nodiscard]]
    constexpr auto operator()(From from) const noexcept -> To
    {
        return static_cast<To>(from);
    }
};

template <integral To>
struct overflowing_cast_fn {
    template <integral From>
    [[nodiscard]]
    constexpr auto operator()(From from) const noexcept -> overflow_result<To>
    {
        if constexpr (requires { To{from}; }) {
            return {To{from}, false}; // not a narrowing conversion
        } else {
            return {static_cast<To>(from), !std::in_range<To>(from)};
        }
    }
};

template <integral To>
struct checked_cast_fn {
    template <integral From>
    [[nodiscard]]
    constexpr auto operator()(From from,
                              std::source_location loc = std::source_location::current()) const
        -> To
    {
        auto r = overflowing_cast_fn<To>{}(from);
        if (r.overflowed) {
            runtime_error("checked_cast failed", loc);
        }
        return r.value;
    }
};

template <integral To>
struct cast_fn {
    template <integral From>
    [[nodiscard]]
    constexpr auto operator()(From from,
                              std::source_location loc = std::source_location::current()) const
        -> To
    {
        if constexpr (config::on_integer_cast == integer_cast_policy::checked) {
            return checked_cast_fn<To>{}(from, loc);
        } else {
            return unchecked_cast_fn<To>{}(from);
        }
    }
};

struct unchecked_add_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs) const noexcept -> T
    {
        return static_cast<T>(lhs + rhs);
    }
};

struct unchecked_sub_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs) const noexcept -> T
    {
        return static_cast<T>(lhs - rhs);
    }
};

struct unchecked_mul_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs) const noexcept -> T
    {
        return static_cast<T>(lhs * rhs);
    }
};

struct unchecked_div_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs) const noexcept -> T
    {
        return static_cast<T>(lhs / rhs);
    }
};

struct unchecked_mod_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs) const noexcept -> T
    {
        return static_cast<T>(lhs % rhs);
    }
};

struct unchecked_shl_fn {
    template <integral T, integral U>
    [[nodiscard]]
    constexpr auto operator()(T lhs, U rhs) const noexcept -> T
    {
        return static_cast<T>(lhs << rhs);
    }
};

struct unchecked_shr_fn {
    template <integral T, integral U>
    [[nodiscard]]
    constexpr auto operator()(T lhs, U rhs) const noexcept -> T
    {
        return static_cast<T>(lhs >> rhs);
    }
};

struct unchecked_neg_fn {
    template <signed_integral T>
    [[nodiscard]]
    constexpr auto operator()(T val) const noexcept -> T
    {
        return static_cast<T>(-val);
    }
};

struct wrapping_add_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs) const noexcept -> T
    {
        using U = std::make_unsigned_t<T>;
        return static_cast<T>(static_cast<U>(lhs) + static_cast<U>(rhs));
    }
};

struct wrapping_sub_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs) const noexcept -> T
    {
        using U = std::make_unsigned_t<T>;
        return static_cast<T>(static_cast<U>(lhs) - static_cast<U>(rhs));
    }
};

struct wrapping_mul_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs) const noexcept -> T
    {
        using U = std::conditional_t<(sizeof(T) < sizeof(unsigned)),
                                     unsigned,
                                     std::make_unsigned_t<T>>;
        return static_cast<T>(static_cast<U>(lhs) * static_cast<U>(rhs));
    }
};

struct wrapping_neg_fn {
    template <signed_integral T>
    [[nodiscard]]
    constexpr auto operator()(T val) const noexcept -> T
    {
        using U = std::make_unsigned_t<T>;
        return static_cast<T>(static_cast<U>(0) - static_cast<U>(val));
    }
};

struct overflowing_add_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs) const noexcept -> overflow_result<T>
    {
        T r;
        bool o = ckd_add(&r, lhs, rhs);
        return {r, o};
    }
};

struct overflowing_sub_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs) const noexcept -> overflow_result<T>
    {
        T r;
        bool o = ckd_sub(&r, lhs, rhs);
        return {r, o};
    }
};

struct overflowing_mul_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs) const noexcept -> overflow_result<T>
    {
        T r;
        bool o = ckd_mul(&r, lhs, rhs);
        return {r, o};
    }
};

struct overflowing_neg_fn {
    template <signed_integral T>
    [[nodiscard]]
    constexpr auto operator()(T val) const noexcept -> overflow_result<T>
    {
        T r;
        bool o = ckd_sub(&r, T{0}, val);
        return {r, o};
    }
};

struct checked_add_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs,
                              std::source_location loc = std::source_location::current()) const
        -> T
    {
        // For built-in signed types at least as large as int,
        // constant evaluation already checks for overflow
        if (signed_integral<T> && (sizeof(T) >= sizeof(int)) &&
            std::is_constant_evaluated()) {
            return unchecked_add_fn{}(lhs, rhs); // LCOV_EXCL_LINE
        } else {
            auto result = overflowing_add_fn{}(lhs, rhs);
            if (result.overflowed) {
                runtime_error("overflow in addition", loc);
            }
            return result.value;
        }
    }
};

struct checked_sub_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs,
                              std::source_location loc = std::source_location::current()) const
        -> T
    {
        if (signed_integral<T> && (sizeof(T) >= sizeof(int)) &&
            std::is_constant_evaluated()) {
            return unchecked_sub_fn{}(lhs, rhs); // LCOV_EXCL_LINE
        } else {
            auto result = overflowing_sub_fn{}(lhs, rhs);
            if (result.overflowed) {
                runtime_error("overflow in subtraction", loc);
            }
            return result.value;
        }
    }
};

struct checked_mul_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs,
                              std::source_location loc = std::source_location::current()) const
        -> T
    {
        if (signed_integral<T> && (sizeof(T) >= sizeof(int)) &&
            std::is_constant_evaluated()) {
            return unchecked_mul_fn{}(lhs, rhs); // LCOV_EXCL_LINE
        } else {
            auto result = overflowing_mul_fn{}(lhs, rhs);
            if (result.overflowed) {
                runtime_error("overflow in multiplication", loc);
            }
            return result.value;
        }
    }
};

template <overflow_policy OnOverflow = overflow_policy::error,
          divide_by_zero_policy OnDivByZero = divide_by_zero_policy::error>
struct checked_div_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs,
                              std::source_location loc = std::source_location::current()) const
        -> T
    {
        // If we're in constant evaluation, we already get a divide-by-zero check
        if (!std::is_constant_evaluated()) {
            if constexpr (OnDivByZero == divide_by_zero_policy::error) {
                if (rhs == T{}) {
                    runtime_error("division by zero", loc);
                }
            }
        }

        // For signed types, MIN/-1 overflows
        if constexpr (signed_integral<T> && (OnOverflow != overflow_policy::ignore)) {
             if (lhs == std::numeric_limits<T>::lowest() && rhs == T{-1}) {
                 runtime_error("overflow in division", loc);
             }
         }

        return unchecked_div_fn{}(lhs, rhs);
    }
};

template <overflow_policy OnOverflow = overflow_policy::error,
          divide_by_zero_policy OnDivByZero = divide_by_zero_policy::error>
struct checked_mod_fn {
    template <integral T>
    [[nodiscard]]
    constexpr auto operator()(T lhs, T rhs,
                              std::source_location loc = std::source_location::current()) const
        -> T
    {
        if (!std::is_constant_evaluated()) {
             if constexpr (OnDivByZero == divide_by_zero_policy::error) {
                 if (rhs == T{}) {
                    runtime_error("modulo with zero", loc);
                 }
             }
        }

        if constexpr (signed_integral<T> && (OnOverflow != overflow_policy::ignore)) {
            if (lhs == std::numeric_limits<T>::lowest() && rhs == T{-1}) {
                runtime_error("overflow in modulo", loc);
            }
        }

        return unchecked_mod_fn{}(lhs, rhs);
    }
};

struct checked_shl_fn {
    template <integral T, integral U>
    [[nodiscard]]
    constexpr auto operator()(T lhs, U rhs,
                              std::source_location loc = std::source_location::current()) const
        -> T
    {
        constexpr std::size_t width = sizeof(T) * CHAR_BIT;
        // If T is at least as large as int, we already get a check when
        // in constant evaluation
        if ((!std::is_constant_evaluated() || (sizeof(T) < sizeof(int))) &&
            ((static_cast<std::size_t>(rhs) >= width) || rhs < U{})) {
            flux::runtime_error("left shift argument too large or negative", loc);
        }
        return unchecked_shl_fn{}(lhs, rhs);
    }
};

struct checked_shr_fn {
    template <integral T, integral U>
    [[nodiscard]]
    constexpr auto operator()(T lhs, U rhs,
               std::source_location loc = std::source_location::current()) const
        -> T
    {
        constexpr std::size_t width = sizeof(T) * CHAR_BIT;
        if ((!std::is_constant_evaluated() || (sizeof(T) < sizeof(int)))&&
            ((static_cast<std::size_t>(rhs) >= width) || rhs < U{})) {
            flux::runtime_error("right shift argument too large or negative", loc);
        }
        return unchecked_shr_fn{}(lhs, rhs);
    }
};

struct checked_neg_fn {
    template <signed_integral T>
    [[nodiscard]]
    constexpr auto operator()(T val,
                              std::source_location loc = std::source_location::current()) const
        -> T
    {
        auto [r, o] = overflowing_neg_fn{}(val);
        if (o) {
            flux::runtime_error("Overflow in signed negation", loc);
        }
        return r;
    }
};

template <overflow_policy>
struct default_ops;

template <>
struct default_ops<overflow_policy::ignore> {
    using add_fn = unchecked_add_fn;
    using sub_fn = unchecked_sub_fn;
    using mul_fn = unchecked_mul_fn;
    using neg_fn = unchecked_neg_fn;
    using shl_fn = unchecked_shl_fn;
    using shr_fn = unchecked_shr_fn;
};

template <>
struct default_ops<overflow_policy::wrap> {
    using add_fn = wrapping_add_fn;
    using sub_fn = wrapping_sub_fn;
    using mul_fn = wrapping_mul_fn;
    using neg_fn = wrapping_neg_fn;
    // no wrapping versions of these yet, so use the checked versions
    using shl_fn = checked_shl_fn;
    using shr_fn = checked_shr_fn;
};

template <>
struct default_ops<overflow_policy::error> {
    using add_fn = checked_add_fn;
    using sub_fn = checked_sub_fn;
    using mul_fn = checked_mul_fn;
    using neg_fn = checked_neg_fn;
    using shl_fn = checked_shl_fn;
    using shr_fn = checked_shr_fn;
};

} // namespace detail

FLUX_EXPORT
template <integral To>
inline constexpr auto unchecked_cast = detail::unchecked_cast_fn<To>{};

FLUX_EXPORT
template <integral To>
inline constexpr auto overflowing_cast = detail::overflowing_cast_fn<To>{};

FLUX_EXPORT
template <integral To>
inline constexpr auto checked_cast = detail::checked_cast_fn<To>{};

FLUX_EXPORT
template <integral To>
inline constexpr auto cast = detail::cast_fn<To>{};

FLUX_EXPORT inline constexpr auto unchecked_add = detail::unchecked_add_fn{};
FLUX_EXPORT inline constexpr auto unchecked_sub = detail::unchecked_sub_fn{};
FLUX_EXPORT inline constexpr auto unchecked_mul = detail::unchecked_mul_fn{};
FLUX_EXPORT inline constexpr auto unchecked_div = detail::unchecked_div_fn{};
FLUX_EXPORT inline constexpr auto unchecked_mod = detail::unchecked_mod_fn{};
FLUX_EXPORT inline constexpr auto unchecked_neg = detail::unchecked_neg_fn{};
FLUX_EXPORT inline constexpr auto unchecked_shl = detail::unchecked_shl_fn{};
FLUX_EXPORT inline constexpr auto unchecked_shr = detail::unchecked_shr_fn{};

FLUX_EXPORT inline constexpr auto wrapping_add = detail::wrapping_add_fn{};
FLUX_EXPORT inline constexpr auto wrapping_sub = detail::wrapping_sub_fn{};
FLUX_EXPORT inline constexpr auto wrapping_mul = detail::wrapping_mul_fn{};
FLUX_EXPORT inline constexpr auto wrapping_neg = detail::wrapping_neg_fn{};

FLUX_EXPORT inline constexpr auto overflowing_add = detail::overflowing_add_fn{};
FLUX_EXPORT inline constexpr auto overflowing_sub = detail::overflowing_sub_fn{};
FLUX_EXPORT inline constexpr auto overflowing_mul = detail::overflowing_mul_fn{};
FLUX_EXPORT inline constexpr auto overflowing_neg = detail::overflowing_neg_fn{};

FLUX_EXPORT inline constexpr auto checked_add = detail::checked_add_fn{};
FLUX_EXPORT inline constexpr auto checked_sub = detail::checked_sub_fn{};
FLUX_EXPORT inline constexpr auto checked_mul = detail::checked_mul_fn{};
FLUX_EXPORT inline constexpr auto checked_div = detail::checked_div_fn{};
FLUX_EXPORT inline constexpr auto checked_mod = detail::checked_mod_fn{};
FLUX_EXPORT inline constexpr auto checked_neg = detail::checked_neg_fn{};
FLUX_EXPORT inline constexpr auto checked_shl = detail::checked_shl_fn{};
FLUX_EXPORT inline constexpr auto checked_shr = detail::checked_shr_fn{};

FLUX_EXPORT inline constexpr auto add = detail::default_ops<config::on_overflow>::add_fn{};
FLUX_EXPORT inline constexpr auto sub = detail::default_ops<config::on_overflow>::sub_fn{};
FLUX_EXPORT inline constexpr auto mul = detail::default_ops<config::on_overflow>::mul_fn{};
FLUX_EXPORT inline constexpr auto div =
    detail::checked_div_fn<config::on_overflow, config::on_divide_by_zero>{};
FLUX_EXPORT inline constexpr auto mod =
    detail::checked_mod_fn<config::on_overflow, config::on_divide_by_zero>{};
FLUX_EXPORT inline constexpr auto neg = detail::default_ops<config::on_overflow>::neg_fn{};
FLUX_EXPORT inline constexpr auto shl = detail::default_ops<config::on_overflow>::shl_fn{};
FLUX_EXPORT inline constexpr auto shr = detail::default_ops<config::on_overflow>::shr_fn{};

} // namespace flux::num

#endif
