
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_UTILS_HPP_INCLUDED
#define FLUX_CORE_UTILS_HPP_INCLUDED

#include <compare>
#include <concepts>
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

} // namespace flux

#endif
