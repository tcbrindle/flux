
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_UTILS_HPP_INCLUDED
#define FLUX_CORE_UTILS_HPP_INCLUDED

#include <compare>
#include <concepts>
#include <functional>
#include <type_traits>

#include <flux/core/assert.hpp>

namespace flux {

/*
 * Useful helpers
 */
FLUX_EXPORT
template <typename From, typename To>
concept decays_to = std::same_as<std::remove_cvref_t<From>, To>;

FLUX_EXPORT
template <typename T, typename U>
concept same_decayed = std::same_as<std::remove_cvref_t<T>,
                                    std::remove_cvref_t<U>>;

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

FLUX_EXPORT inline constexpr auto copy = detail::copy_fn{};

struct immovable {
    immovable() = default;
    ~immovable() = default;
    immovable(immovable const&) = delete;
    immovable(immovable&&) = delete;
    immovable& operator=(immovable const&) = delete;
    immovable& operator=(immovable&&) = delete;
};

namespace detail {

template <typename T, typename... U>
concept any_of = (std::same_as<T, U> || ...);

template <typename T, typename Cat>
concept compares_as = std::same_as<std::common_comparison_category_t<T, Cat>, Cat>;

template <typename Fn, typename T, typename U, typename Cat>
concept ordering_invocable_ =
    std::regular_invocable<Fn, T, U> &&
    compares_as<std::decay_t<std::invoke_result_t<Fn, T, U>>, Cat>;

} // namespace detail

FLUX_EXPORT
template <typename Fn, typename T, typename U, typename Cat = std::partial_ordering>
concept ordering_invocable =
    detail::ordering_invocable_<Fn, T, U, Cat> &&
    detail::ordering_invocable_<Fn, U, T, Cat> &&
    detail::ordering_invocable_<Fn, T, T, Cat> &&
    detail::ordering_invocable_<Fn, U, U, Cat>;

FLUX_EXPORT template <typename Fn, typename... Args>
using callable_result_t = decltype(std::declval<Fn>()(std::declval<Args>()...));

namespace detail {

template <typename Fn, typename... Args>
concept can_call_ = requires { typename callable_result_t<Fn, Args...>; };

template <typename Fn, typename R, typename... Args>
concept can_call_r
    = can_call_<Fn, Args...> && std::convertible_to<callable_result_t<Fn, Args...>, R>;

template <typename, typename>
inline constexpr bool callable_ = false;

template <typename Fn, typename R, typename... Args>
inline constexpr bool callable_<Fn, R(Args...)>
    = std::is_void_v<R> ? can_call_<Fn, Args...> : can_call_r<Fn, R, Args...>;

} // namespace detail

FLUX_EXPORT template <typename Fn, typename Sig>
concept callable_once = detail::callable_<Fn, Sig>;

FLUX_EXPORT template <typename Fn, typename Sig>
concept callable_mut = detail::callable_<Fn&, Sig> && callable_once<Fn, Sig>;

FLUX_EXPORT template <typename Fn, typename Sig>
concept callable = detail::callable_<Fn const&, Sig> && callable_mut<Fn, Sig>;

namespace detail {

template <callable_once<void()> Fn>
struct [[nodiscard("Discarded defer_t will execute its associated function immediately")]] defer_t {
    FLUX_NO_UNIQUE_ADDRESS Fn fn;

    constexpr ~defer_t() { fn(); }
};

template <typename F>
defer_t(F) -> defer_t<F>;

template <typename Fn>
constexpr auto copy_or_ref(Fn& fn)
{
    if constexpr (std::is_trivially_copyable_v<Fn> && sizeof(Fn) <= sizeof(void*)) {
        return fn;
    } else {
        return std::ref(fn);
    }
}

template <typename Fn>
using copy_or_ref_t = decltype(copy_or_ref(std::declval<Fn&>()));

template <typename Fn>
struct emplace_from {
    Fn fn;
    using type = decltype(std::move(fn)());

    constexpr operator type() && noexcept { return std::move(fn)(); }

    constexpr type operator()() && { return std::move(fn)(); }
};

template <typename Fn>
emplace_from(Fn) -> emplace_from<Fn>;

[[noreturn]] inline void unreachable()
{
    if constexpr (config::enable_debug_asserts) {
        runtime_error(
            "Unreachable code reached! This should never happen. Please file a bug report.");
    }
#if defined(__cpp_lib_unreachable) && (__cpp_lib_unreachable >= 202202L)
    std::unreachable();
#elif defined(__has_builtin) && __has_builtin(__builtin_unreachable)
    __builtin_unreachable();
#elif defined(_MSC_VER)
    __assume(false);
#else
    std::abort();
#endif
}

} // namespace detail

} // namespace flux

#endif
