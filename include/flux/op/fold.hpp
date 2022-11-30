
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FOLD_HPP_INCLUDED
#define FLUX_OP_FOLD_HPP_INCLUDED

#include <flux/op/for_each_while.hpp>

namespace flux {

namespace detail {

template <typename Seq, typename Func, typename Init>
using fold_result_t = std::decay_t<std::invoke_result_t<Func&, Init, element_t<Seq>>>;

struct fold_op {
    template <sequence Seq, typename Func, std::movable Init = value_t<Seq>,
              typename R = fold_result_t<Seq, Func, Init>>
        requires std::invocable<Func&,  Init, element_t<Seq>> &&
                 std::invocable<Func&, R, element_t<Seq>> &&
                 std::convertible_to<Init, R> &&
                 std::assignable_from<Init&, std::invoke_result_t<Func&, R, element_t<Seq>>>
    constexpr auto operator()(Seq&& seq, Func func, Init init = Init{}) const -> R
    {
        R init_ = std::move(init);
        flux::for_each_while(seq, [&func, &init_](auto&& elem) {
            init_ = std::invoke(func, std::move(init_), FLUX_FWD(elem));
            return true;
        });
        return init_;
    }
};

} // namespace detail

inline constexpr auto fold = detail::fold_op{};

template <typename Derived>
template <typename D, typename Func, typename Init, typename R>
    requires std::invocable<Func&, Init, element_t<D>> &&
             std::invocable<Func&, R, element_t<D>> &&
             std::convertible_to<R, Init> &&
             std::assignable_from<Init&, std::invoke_result_t<Func&, R, element_t<Derived>>>
[[nodiscard]]
constexpr auto lens_base<Derived>::fold(Func func, Init init) -> R
{
    return flux::fold(derived(), std::move(func), std::move(init));
}

} // namespace flux

#endif // FLUX_OP_FOLD_HPP_INCLUDED
