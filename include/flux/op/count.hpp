
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_COUNT_HPP_INCLUDED
#define FLUX_OP_COUNT_HPP_INCLUDED

#include <flux/op/for_each_while.hpp>

namespace flux {

namespace detail {

struct count_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> distance_t
    {
        if constexpr (sized_sequence<Seq>) {
            return flux::size(seq);
        } else {
            distance_t counter = 0;
            flux::for_each_while(seq, [&](auto&&) {
                ++counter;
                return true;
            });
            return counter;
        }
    }
};

struct count_eq_fn {
    template <sequence Seq, typename Value, typename Proj = std::identity>
        requires std::equality_comparable_with<projected_t<Proj, Seq>, Value const&>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Value const& value, Proj proj = {}) const
        -> distance_t
    {
        distance_t counter = 0;
        flux::for_each_while(seq, [&](auto&& elem) {
            if (value == std::invoke(proj, FLUX_FWD(elem))) {
                ++counter;
            }
            return true;
        });
        return counter;
    }
};

struct count_if_fn {
    template <sequence Seq, typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Seq, Proj>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred, Proj proj = {}) const
        -> distance_t
    {
        distance_t counter = 0;
        flux::for_each_while(seq, [&](auto&& elem) {
            if (std::invoke(pred, std::invoke(proj, FLUX_FWD(elem)))) {
                ++counter;
            }
            return true;
        });
        return counter;
    }
};

} // namespace detail

inline constexpr auto count = detail::count_fn{};
inline constexpr auto count_eq = detail::count_eq_fn{};
inline constexpr auto count_if = detail::count_if_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::count()
{
    return flux::count(derived());
}

template <typename D>
template <typename Value, typename Proj>
    requires std::equality_comparable_with<projected_t<Proj, D>, Value const&>
constexpr auto inline_sequence_base<D>::count_eq(Value const& value, Proj proj)
{
    return flux::count_eq(derived(), value, std::move(proj));
}

template <typename D>
template <typename Pred, typename Proj>
    requires predicate_for<Pred, D, Proj>
constexpr auto inline_sequence_base<D>::count_if(Pred pred, Proj proj)
{
    return flux::count_if(derived(), std::move(pred), std::move(proj));
}

} // namespace flux

#endif
