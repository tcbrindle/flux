
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_ALL_ANY_NONE_HPP_INCLUDED
#define FLUX_ALGORITHM_ALL_ANY_NONE_HPP_INCLUDED

#include <flux/algorithm/for_each_while.hpp>

namespace flux {

FLUX_EXPORT
struct all_t {
    template <iterable It, typename Pred>
        requires std::predicate<Pred const&, iterable_element_t<It>>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Pred const pred) const -> bool
    {
        return for_each_while(it, [&](auto&& elem) { return std::invoke(pred, FLUX_FWD(elem)); })
            == iteration_result::complete;
    }
};

FLUX_EXPORT inline constexpr all_t all {};

FLUX_EXPORT
struct none_t {
    template <iterable It, typename Pred>
        requires std::predicate<Pred&, iterable_element_t<It>>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Pred const pred) const -> bool
    {
        return for_each_while(it, [&](auto&& elem) { return !std::invoke(pred, FLUX_FWD(elem)); })
            == iteration_result::complete;
    }
};

FLUX_EXPORT inline constexpr none_t none {};

FLUX_EXPORT
struct any_t {
    template <iterable It, typename Pred>
        requires std::predicate<Pred&, iterable_element_t<It>>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Pred const pred) const -> bool
    {
        return for_each_while(it, [&](auto&& elem) { return !std::invoke(pred, FLUX_FWD(elem)); })
            == iteration_result::incomplete;
    }
};

FLUX_EXPORT inline constexpr any_t any {};

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
