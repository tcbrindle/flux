
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FIND_HPP_INCLUDED
#define FLUX_OP_FIND_HPP_INCLUDED

#include <flux/op/for_each_while.hpp>

namespace flux {

namespace detail {

struct find_fn {
    template <sequence Seq, typename Value>
        requires std::equality_comparable_with<element_t<Seq>, Value const&>
    constexpr auto operator()(Seq&& seq, Value const& value) const -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return FLUX_FWD(elem) != value;
        });
    }
};

struct find_if_fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr auto operator()(Seq&& seq, Pred pred) const
        -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, FLUX_FWD(elem));
        });
    }
};

struct find_if_not_fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr auto operator()(Seq&& seq, Pred pred) const
        -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return std::invoke(pred, FLUX_FWD(elem));
        });
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto find = detail::find_fn{};
FLUX_EXPORT inline constexpr auto find_if = detail::find_if_fn{};
FLUX_EXPORT inline constexpr auto find_if_not = detail::find_if_not_fn{};

template <typename D>
template <typename Value>
    requires std::equality_comparable_with<element_t<D>, Value const&>
constexpr auto inline_sequence_base<D>::find(Value const& val)
{
    return flux::find(derived(), val);
}

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::find_if(Pred pred)
{
    return flux::find_if(derived(), std::ref(pred));
}

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::find_if_not(Pred pred)
{
    return flux::find_if_not(derived(), std::ref(pred));
}

} // namespace flux

#endif // FLUX_OP_FIND_HPP_INCLUDED