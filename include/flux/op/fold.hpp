
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FOLD_HPP_INCLUDED
#define FLUX_OP_FOLD_HPP_INCLUDED

#include <flux/op/for_each_while.hpp>

namespace flux {

namespace detail {

struct fold_op {
    template <sequence Seq, typename Func, typename Init = value_t<Seq>>
        requires std::invocable<Func&,  Init, element_t<Seq>> &&
                 std::assignable_from<Init&, std::invoke_result_t<Func&, Init, element_t<Seq>>>
    constexpr auto operator()(Seq&& seq, Func func, Init init = Init{}) const -> Init
    {
        flux::for_each_while(seq, [&](auto&& elem) {
            init = std::invoke(func, std::move(init), FLUX_FWD(elem));
            return true;
        });
        return init;
    }
};

} // namespace detail

inline constexpr auto fold = detail::fold_op{};

template <typename D>
template <typename Func, typename Init>
    requires std::invocable<Func&, Init, element_t<D>> &&
             std::assignable_from<Init&, std::invoke_result_t<Func&, Init, element_t<D>>>
constexpr auto lens_base<D>::fold(Func func, Init init) -> Init
{
    return flux::fold(derived(), std::move(func), std::move(init));
}

} // namespace flux

#endif // FLUX_OP_FOLD_HPP_INCLUDED
