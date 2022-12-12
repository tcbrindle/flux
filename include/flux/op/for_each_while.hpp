
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FOR_EACH_WHILE_HPP_INCLUDED
#define FLUX_OP_FOR_EACH_WHILE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

struct for_each_while_fn {
    template <sequence Seq, typename Pred>
        requires std::invocable<Pred&, element_t<Seq>> &&
                 boolean_testable<std::invoke_result_t<Pred&, element_t<Seq>>>
    constexpr auto operator()(Seq&& seq, Pred pred) const -> cursor_t<Seq>
    {
        if constexpr (requires { traits_t<Seq>::for_each_while(seq, std::move(pred)); }) {
            return traits_t<Seq>::for_each_while(seq, std::move(pred));
        } else {
            auto cur = first(seq);
            while (!is_last(seq, cur)) {
                if (!std::invoke(pred, read_at(seq, cur))) { break; }
                inc(seq, cur);
            }
            return cur;
        }
    }
};

} // namespace detail

inline constexpr auto for_each_while = detail::for_each_while_fn{};

template <typename Derived>
template <typename Pred>
    requires std::invocable<Pred&, element_t<Derived>> &&
             detail::boolean_testable<std::invoke_result_t<Pred&, element_t<Derived>>>
constexpr auto lens_base<Derived>::for_each_while(Pred pred)
{
    return flux::for_each_while(derived(), std::ref(pred));
}

} // namespace flux

#endif // FLUX_OP_FOR_EACH_WHILE_HPP_INCLUDED
