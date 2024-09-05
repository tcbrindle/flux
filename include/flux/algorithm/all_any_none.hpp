
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_ALL_ANY_NONE_HPP_INCLUDED
#define FLUX_ALGORITHM_ALL_ANY_NONE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace all_detail {

struct fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr bool operator()(Seq&& seq, Pred pred) const
    {
        return is_last(seq, for_each_while(seq, [&](auto&& elem) {
            return std::invoke(pred, FLUX_FWD(elem));
        }));
    }
};

} // namespace all_detail

FLUX_EXPORT inline constexpr auto all = all_detail::fn{};

namespace none_detail {

struct fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr bool operator()(Seq&& seq, Pred pred) const
    {
        return is_last(seq, for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, FLUX_FWD(elem));
        }));
    }
};

} // namespace none_detail

FLUX_EXPORT inline constexpr auto none = none_detail::fn{};

namespace any_detail {

struct fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr bool operator()(Seq&& seq, Pred pred) const
    {
        return !is_last(seq, for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, FLUX_FWD(elem));
        }));
    }
};

} // namespace any_detail

FLUX_EXPORT inline constexpr auto any = any_detail::fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::all(Pred pred)
{
    return flux::all(derived(), std::move(pred));
}

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::any(Pred pred)
{
    return flux::any(derived(), std::move(pred));
}

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::none(Pred pred)
{
    return flux::none(derived(), std::move(pred));
}

} // namespace flux

#endif // FLUX_ALGORITHM_ALL_ANY_NONE_HPP_INCLUDED
