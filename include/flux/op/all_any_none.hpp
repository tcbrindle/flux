
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ALL_ANY_NONE_HPP_INCLUDED
#define FLUX_OP_ALL_ANY_NONE_HPP_INCLUDED

#include <flux/core/lens_base.hpp>
#include <flux/op/for_each_while.hpp>

namespace flux {

namespace all_detail {

struct fn {
    template <sequence Seq, typename Proj = std::identity,
              predicate_for<Seq, Proj> Pred>
    constexpr bool operator()(Seq&& seq, Pred pred, Proj proj = {}) const
    {
        return is_last(seq, for_each_while(seq, [&](auto&& elem) {
            return std::invoke(pred, std::invoke(proj, FLUX_FWD(elem)));
        }));
    }
};

} // namespace all_detail

inline constexpr auto all = all_detail::fn{};

namespace none_detail {

struct fn {
    template <sequence Seq, typename Proj = std::identity,
              predicate_for<Seq, Proj> Pred>
    constexpr bool operator()(Seq&& seq, Pred pred, Proj proj = {}) const
    {
        return is_last(seq, for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, std::invoke(proj, FLUX_FWD(elem)));
        }));
    }
};

} // namespace none_detail

inline constexpr auto none = none_detail::fn{};

namespace any_detail {

struct fn {
    template <sequence Seq, typename Proj = std::identity,
              predicate_for<Seq, Proj> Pred>
    constexpr bool operator()(Seq&& seq, Pred pred, Proj proj = {}) const
    {
        return !is_last(seq, for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, std::invoke(proj, FLUX_FWD(elem)));
        }));
    }
};

} // namespace any_detail

inline constexpr auto any = any_detail::fn{};

template <typename D>
template <typename Pred, typename Proj>
    requires predicate_for<Pred, D, Proj>
constexpr auto lens_base<D>::all(Pred pred, Proj proj)
{
    return flux::all(derived(), std::move(pred), std::move(proj));
}

template <typename D>
template <typename Pred, typename Proj>
    requires predicate_for<Pred, D, Proj>
constexpr auto lens_base<D>::any(Pred pred, Proj proj)
{
    return flux::any(derived(), std::move(pred), std::move(proj));
}

template <typename D>
template <typename Pred, typename Proj>
    requires predicate_for<Pred, D, Proj>
constexpr auto lens_base<D>::none(Pred pred, Proj proj)
{
    return flux::none(derived(), std::move(pred), std::move(proj));
}

} // namespace flux

#endif // FLUX_OP_ALL_ANY_NONE_HPP_INCLUDED
