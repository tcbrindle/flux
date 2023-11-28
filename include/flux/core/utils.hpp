
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_UTILS_HPP_INCLUDED
#define FLUX_CORE_UTILS_HPP_INCLUDED

#include <flux/core/assert.hpp>

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
FLUX_EXPORT
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

FLUX_EXPORT inline constexpr auto copy = detail::copy_fn{};

namespace detail {

template <typename T, typename... U>
concept any_of = (std::same_as<T, U> || ...);

} // namespace detail

} // namespace flux

#endif
