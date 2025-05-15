// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_ZIP_ALGORITHMS_HPP_INCLUDED
#define FLUX_ALGORITHM_ZIP_ALGORITHMS_HPP_INCLUDED

#include <flux/algorithm/find.hpp>
#include <flux/algorithm/for_each_while.hpp>

namespace flux {

namespace detail {

struct zip_for_each_while_fn {
    template <typename Pred, sequence... Seqs>
        requires std::invocable<Pred&, element_t<Seqs>...>
        && boolean_testable<std::invoke_result_t<Pred&, element_t<Seqs>...>>
    constexpr auto operator()(Pred pred, Seqs&&... seqs) const -> iteration_result
    {
        if constexpr (sizeof...(Seqs) == 0) {
            return {};
        } else if constexpr (sizeof...(Seqs) == 1) {
            return flux::for_each_while(seqs..., std::ref(pred));
        } else {
            return [&pred, ... ctxs = iterate(seqs)]() mutable {
                return [&pred, &ctxs..., ... opts = step(ctxs, std::identity {})]() mutable {
                    while (true) {
                        if (!(opts.has_value() && ...)) {
                            return iteration_result::complete;
                        }
                        if (!std::invoke(pred, opts.value_unchecked()...)) {
                            return iteration_result::incomplete;
                        }
                        ((opts = step(ctxs, std::identity {})), ...);
                    }
                }();
            }();
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto zip_for_each_while = detail::zip_for_each_while_fn{};

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
        if constexpr (sizeof...(Seqs) == 0) {
            return {};
        } else if constexpr (sizeof...(Seqs) == 1) {
            return std::tuple<cursor_t<Seqs>...>(flux::find_if(seqs..., std::ref(pred)));
        } else {
            return [&pred, &... seqs = seqs, ... curs = first(seqs)]() mutable {
                while ((!is_last(seqs, curs) && ...)) {
                    if (std::invoke(pred, read_at_unchecked(seqs, curs)...)) {
                        break;
                    }
                    ((inc(seqs, curs)), ...);
                }
                return std::tuple<cursor_t<Seqs>...>(std::move(curs)...);
            }();
        }
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

FLUX_EXPORT inline constexpr auto zip_for_each = detail::zip_for_each_fn{};
FLUX_EXPORT inline constexpr auto zip_find_if = detail::zip_find_if_fn{};
FLUX_EXPORT inline constexpr auto zip_fold = detail::zip_fold_fn{};

} // namespace pred

#endif // FLUX_ALGORITHM_ZIP_ALGORITHMS_HPP_INCLUDED
