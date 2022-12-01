
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FOLD_HPP_INCLUDED
#define FLUX_OP_FOLD_HPP_INCLUDED

#include <flux/op/for_each_while.hpp>

#include <optional>

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
        R init_ = R(std::move(init));
        flux::for_each_while(seq, [&func, &init_](auto&& elem) {
            init_ = std::invoke(func, std::move(init_), FLUX_FWD(elem));
            return true;
        });
        return init_;
    }
};

struct fold_first_op {
    template <sequence Seq, typename Func, typename V = value_t<Seq>>
        requires std::invocable<Func&, V, element_t<Seq>> &&
                 std::assignable_from<value_t<Seq>&, std::invoke_result_t<Func&, V&&, element_t<Seq>>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func) const -> std::optional<V>
    {
        auto cur = flux::first(seq);

        if (flux::is_last(seq, cur)) {
            return std::nullopt;
        }

        V init(flux::unchecked_read_at(seq, cur));
        flux::inc(seq, cur);

        while (!flux::is_last(seq, cur)) {
            init = std::invoke(func, std::move(init), flux::unchecked_read_at(seq, cur));
            flux::inc(seq, cur);
        }

        return std::optional<V>(std::in_place, std::move(init));
    }
};

} // namespace detail

inline constexpr auto fold = detail::fold_op{};
inline constexpr auto fold_first = detail::fold_first_op{};

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

template <typename Derived>
template <typename D, typename Func>
    requires std::invocable<Func&, value_t<D>, element_t<D>> &&
             std::assignable_from<value_t<D>&, std::invoke_result_t<Func&, value_t<D>, element_t<D>>>
constexpr auto lens_base<Derived>::fold_first(Func func)
{
    return flux::fold_first(derived(), std::move(func));
}

} // namespace flux

#endif // FLUX_OP_FOLD_HPP_INCLUDED
