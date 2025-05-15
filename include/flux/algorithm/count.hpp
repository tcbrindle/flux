
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_COUNT_HPP_INCLUDED
#define FLUX_ALGORITHM_COUNT_HPP_INCLUDED

#include <flux/algorithm/for_each.hpp>

namespace flux {

FLUX_EXPORT
struct count_t {
    template <iterable It>
    [[nodiscard]]
    constexpr auto operator()(It&& it) const -> int_t
    {
        if constexpr (sized_iterable<It>) {
            return flux::iterable_size(it);
        } else {
            int_t counter = 0;
            for_each(it, [&](auto&&) { counter = num::add(counter, int_t{1}); });
            return counter;
        }
    }
};

FLUX_EXPORT inline constexpr count_t count{};

FLUX_EXPORT
struct count_if_t {
    template <iterable It, typename Pred>
        requires std::predicate<Pred&, iterable_element_t<It>>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Pred pred) const -> int_t
    {
        int_t counter = 0;
        for_each(it, [&](auto&& elem) {
            if (std::invoke(pred, FLUX_FWD(elem))) {
                ++counter;
            }
        });
        return counter;
    }
};

FLUX_EXPORT inline constexpr count_if_t count_if{};

FLUX_EXPORT
struct count_eq_t {
    template <iterable It, typename Value>
        requires std::equality_comparable_with<iterable_element_t<It>, Value const&>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Value const& value) const -> int_t
    {
        return count_if(it, [&](auto&& elem) { return value == FLUX_FWD(elem); });
    }
};

FLUX_EXPORT inline constexpr count_eq_t count_eq{};

template <typename D>
constexpr auto inline_sequence_base<D>::count()
{
    return flux::count(derived());
}

template <typename D>
template <typename Value>
    requires std::equality_comparable_with<element_t<D>, Value const&>
constexpr auto inline_sequence_base<D>::count_eq(Value const& value)
{
    return flux::count_eq(derived(), value);
}

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::count_if(Pred pred)
{
    return flux::count_if(derived(), std::move(pred));
}

} // namespace flux

#endif // FLUX_ALGORITHM_COUNT_HPP_INCLUDED
