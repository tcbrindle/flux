// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_FOR_EACH_WHILE_HPP_INCLUDED
#define FLUX_ALGORITHM_FOR_EACH_WHILE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

FLUX_EXPORT
struct for_each_while_t {
    template <iterable It, callable_mut<bool(iterable_element_t<It>)> Pred>
    constexpr auto operator()(It&& it, Pred&& pred) const -> iteration_result
    {
        iteration_context auto ctx = iterate(it);
        return ctx.run_while(FLUX_FWD(pred));
    }
};

FLUX_EXPORT inline constexpr for_each_while_t for_each_while {};

} // namespace flux

#endif // FLUX_ALGORITHM_FOR_EACH_WHILE_HPP_INCLUDED