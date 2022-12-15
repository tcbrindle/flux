
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FIND_HPP_INCLUDED
#define FLUX_OP_FIND_HPP_INCLUDED

#include <flux/op/for_each_while.hpp>

namespace flux {

namespace detail {

struct find_fn {
    template <sequence Seq, typename Value, typename Proj = std::identity>
        requires std::equality_comparable_with<projected_t<Proj, Seq>, Value const&>
    constexpr auto operator()(Seq&& seq, Value const& value,
                              Proj proj = {}) const -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return std::invoke(proj, FLUX_FWD(elem)) != value;
        });
    }
};

struct find_if_fn {
    template <sequence Seq, typename Pred, typename Proj = std::identity>
        requires std::predicate<Pred&, projected_t<Proj, Seq>>
    constexpr auto operator()(Seq&& seq, Pred pred, Proj proj = {}) const
        -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, std::invoke(proj, FLUX_FWD(elem)));
        });
    }
};

struct find_if_not_fn {
    template <sequence Seq, typename Pred, typename Proj = std::identity>
        requires std::predicate<Pred&, projected_t<Proj, Seq>>
    constexpr auto operator()(Seq&& seq, Pred pred, Proj proj = {}) const
        -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return std::invoke(pred, std::invoke(proj, FLUX_FWD(elem)));
        });
    }
};

} // namespace detail

inline constexpr auto find = detail::find_fn{};
inline constexpr auto find_if = detail::find_if_fn{};
inline constexpr auto find_if_not = detail::find_if_not_fn{};

template <typename D>
template <typename Value, typename Proj>
    requires std::equality_comparable_with<projected_t<Proj, D>, Value const&>
constexpr auto inline_sequence_base<D>::find(Value const& val, Proj proj)
{
    return flux::find(derived(), val, std::ref(proj));
}

template <typename D>
template <typename Pred, typename Proj>
    requires predicate_for<Pred, D, Proj>
constexpr auto inline_sequence_base<D>::find_if(Pred pred, Proj proj)
{
    return flux::find_if(derived(), std::ref(pred), std::ref(proj));
}

template <typename D>
template <typename Pred, typename Proj>
    requires predicate_for<Pred, D, Proj>
constexpr auto inline_sequence_base<D>::find_if_not(Pred pred, Proj proj)
{
    return flux::find_if_not(derived(), std::ref(pred), std::ref(proj));
}

} // namespace flux

#endif // FLUX_OP_FIND_HPP_INCLUDED