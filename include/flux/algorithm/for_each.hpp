
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_FOR_EACH_HPP_INCLUDED
#define FLUX_ALGORITHM_FOR_EACH_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

struct for_each_fn {

    template <sequence Seq, typename Func>
        requires (std::invocable<Func&, element_t<Seq>> &&
                  !infinite_sequence<Seq>)
    constexpr auto operator()(Seq&& seq, Func func) const -> Func
    {
        (void)flux::seq_for_each_while(FLUX_FWD(seq), [&](auto&& elem) {
            std::invoke(func, FLUX_FWD(elem));
            return true;
        });
        return func;
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto for_each = detail::for_each_fn{};

template <typename D>
template <typename Func>
    requires std::invocable<Func&, element_t<D>>
constexpr auto inline_sequence_base<D>::for_each(Func func) -> Func
{
    return flux::for_each(derived(), std::move(func));
}

} // namespace flux

#endif // FLUX_ALGORITHM_FOR_EACH_HPP_INCLUDED
