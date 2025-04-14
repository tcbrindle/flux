
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_CONTAINS_HPP_INCLUDED
#define FLUX_ALGORITHM_CONTAINS_HPP_INCLUDED

#include <flux/algorithm/all_any_none.hpp>

namespace flux {

namespace detail {

struct contains_fn {
    template <iterable It, typename Value>
        requires std::equality_comparable_with<element_t<It>, Value const&>
    constexpr auto operator()(It&& it, Value const& value) const -> bool
    {
        return any(it, [&](auto&& elem) { return FLUX_FWD(elem) == value; });
    }
};


} // namespace detail

FLUX_EXPORT inline constexpr auto contains = detail::contains_fn{};

template <typename D>
template <typename Value>
    requires std::equality_comparable_with<element_t<D>, Value const&>
constexpr auto inline_iter_base<D>::contains(Value const& value) -> bool
{
    return flux::contains(derived(), value);
}

} // namespace flux

#endif // FLUX_ALGORITHM_CONTAINS_HPP_INCLUDED
