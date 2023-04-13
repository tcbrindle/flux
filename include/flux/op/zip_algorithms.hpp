// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ZIP_ALGORITHMS_HPP_INCLUDED
#define FLUX_OP_ZIP_ALGORITHMS_HPP_INCLUDED

#include <flux/core.hpp>

#include <flux/op/for_each_while.hpp>

namespace flux {

namespace detail {

struct zip_for_each_while_fn {
    template <typename Pred, sequence... Seqs>
        requires std::invocable<Pred&, element_t<Seqs>...> &&
                 boolean_testable<std::invoke_result_t<Pred&, element_t<Seqs>...>>
    constexpr auto operator()(Pred pred, Seqs&&... seqs) const
        -> std::tuple<cursor_t<Seqs>...>
    {
        if constexpr (sizeof...(Seqs) == 0) {
            return std::tuple<>{};
        } else if constexpr (sizeof...(Seqs) == 1) {
            return std::tuple<cursor_t<Seqs>...>(flux::for_each_while(seqs..., std::ref(pred)));
        } else {
            return [&pred, &...seqs = seqs, ...curs = flux::first(seqs)]() mutable {
                while (!(flux::is_last(seqs, curs) || ...)) {
                    if (!std::invoke(pred, flux::read_at_unchecked(seqs, curs)...)) {
                        break;
                    }
                    (flux::inc(seqs, curs), ...);
                }
                return std::tuple<cursor_t<Seqs>...>(std::move(curs)...);
            }();
        }
    }
};

} // namespace detail

inline constexpr auto zip_for_each_while = detail::zip_for_each_while_fn{};

namespace detail {

struct zip_for_each_fn {
    template <std::move_constructible Func, sequence... Seqs>
        requires std::invocable<Func&, element_t<Seqs>...>
    constexpr auto operator()(Func func, Seqs&&... seqs) const -> Func
    {
        zip_for_each_while([&func](auto&&... elems) {
            std::invoke(func, FLUX_FWD(elems)...);
            return true;
        }, seqs...);
        return func;
    }
};

struct zip_find_if_fn {
    template <typename Pred, sequence... Seqs>
        requires std::predicate<Pred&, element_t<Seqs>...>
    constexpr auto operator()(Pred pred, Seqs&&... seqs) const
        -> std::tuple<cursor_t<Seqs>...>
    {
        return zip_for_each_while(std::not_fn(pred), seqs...);
    }
};

template <typename Func, typename Init, typename... Seqs>
using zip_fold_result_t = std::decay_t<std::invoke_result_t<Func&, Init, element_t<Seqs>...>>;

template <typename Func, typename Init, typename R, typename... Seqs>
concept zip_foldable =
    std::invocable<Func&, R, element_t<Seqs>...> &&
    std::convertible_to<Init, R> &&
    std::assignable_from<R&, std::invoke_result_t<Func&, R, element_t<Seqs>...>>;

struct zip_fold_fn {
    template <typename Func, std::movable Init, sequence... Seqs,
              typename R = zip_fold_result_t<Func, Init, Seqs...>>
        requires zip_foldable<Func, Init, R, Seqs...>
    constexpr auto operator()(Func func, Init init, Seqs&&... seqs) const -> R
    {
        R init_ = R(std::move(init));
        zip_for_each_while([&func, &init_](auto&&... elems) {
            init_ = std::invoke(func, std::move(init_), FLUX_FWD(elems)...);
            return true;
        }, seqs...);
        return init_;
    }
};

} // namespace detail

inline constexpr auto zip_for_each = detail::zip_for_each_fn{};
inline constexpr auto zip_find_if = detail::zip_find_if_fn{};
inline constexpr auto zip_fold = detail::zip_fold_fn{};

} // namespace pred

#endif
