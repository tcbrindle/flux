
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_FOR_EACH_HPP_INCLUDED
#define FLUX_ALGORITHM_FOR_EACH_HPP_INCLUDED

#include <flux/algorithm/for_each_while.hpp>

namespace flux {

FLUX_EXPORT
struct for_each_t {
    template <iterable It, typename Func>
        requires std::invocable<Func&, iterable_element_t<It>>
    constexpr auto operator()(It&& it, Func func) const -> Func
    {
        for_each_while(it, [&](auto&& elem) {
            static_cast<void>(std::invoke(func, FLUX_FWD(elem)));
            return loop_continue;
        });
        return func;
    }
};

FLUX_EXPORT inline constexpr for_each_t for_each {};

template <typename D>
template <typename Func>
    requires std::invocable<Func&, element_t<D>>
constexpr auto inline_sequence_base<D>::for_each(Func func) -> Func
{
    return flux::for_each(derived(), std::move(func));
}

} // namespace flux

#endif // FLUX_ALGORITHM_FOR_EACH_HPP_INCLUDED
