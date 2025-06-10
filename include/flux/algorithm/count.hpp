
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_COUNT_HPP_INCLUDED
#define FLUX_ALGORITHM_COUNT_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

struct count_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> int_t
    {
        if constexpr (sized_sequence<Seq>) {
            return flux::size(seq);
        } else {
            int_t counter = 0;
            flux::seq_for_each_while(seq, [&](auto&&) {
                ++counter;
                return true;
            });
            return counter;
        }
    }
};

struct count_eq_fn {
    template <sequence Seq, typename Value>
        requires std::equality_comparable_with<element_t<Seq>, Value const&>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Value const& value) const -> int_t
    {
        int_t counter = 0;
        flux::seq_for_each_while(seq, [&](auto&& elem) {
            if (value == FLUX_FWD(elem)) {
                ++counter;
            }
            return true;
        });
        return counter;
    }
};

struct count_if_fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred) const -> int_t
    {
        int_t counter = 0;
        flux::seq_for_each_while(seq, [&](auto&& elem) {
            if (std::invoke(pred, FLUX_FWD(elem))) {
                ++counter;
            }
            return true;
        });
        return counter;
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto count = detail::count_fn{};
FLUX_EXPORT inline constexpr auto count_eq = detail::count_eq_fn{};
FLUX_EXPORT inline constexpr auto count_if = detail::count_if_fn{};

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
