
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_MINMAX_HPP_INCLUDED
#define FLUX_ALGORITHM_MINMAX_HPP_INCLUDED

#include <flux/core.hpp>

#include <flux/algorithm/fold.hpp>

namespace flux {

FLUX_EXPORT
template <typename T>
struct minmax_result {
    T min;
    T max;
};

namespace detail {

struct min_op {
    template <iterable It, weak_ordering_for<It> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Cmp cmp = Cmp{}) const
        -> flux::optional<value_t<It>>
    {
        return flux::fold_first(it, [&](auto min, auto&& elem) -> value_t<It> {
            if (std::invoke(cmp, elem, min) < 0) {
                return value_t<It>(FLUX_FWD(elem));
            } else {
                return min;
            }
        });
    }
};

struct max_op {
    template <iterable It, weak_ordering_for<It> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Cmp cmp = Cmp{}) const
        -> flux::optional<value_t<It>>
    {
        return flux::fold_first(it, [&](auto max, auto&& elem) -> value_t<It> {
            if (!(std::invoke(cmp, elem, max) < 0)) {
                return value_t<It>(FLUX_FWD(elem));
            } else {
                return max;
            }
        });
    }
};

struct minmax_op {
    template <iterable It, weak_ordering_for<It> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(It&& seq, Cmp cmp = Cmp{}) const
        -> flux::optional<minmax_result<value_t<It>>>
    {
        using Opt = optional<minmax_result<value_t<It>>>;

        auto fold_fn = [&cmp](Opt o, auto&& elem) -> Opt {
            if (o.has_value()) {
                auto& mm = o.value_unchecked();
                if (std::invoke(cmp, elem, mm.min) < 0) {
                    mm.min = value_t<It>(elem);
                }
                if (!(std::invoke(cmp, elem, mm.max) < 0)) {
                    mm.max = value_t<It>(FLUX_FWD(elem));
                }
            } else {
                o.emplace(value_t<It>(elem), value_t<It>(elem));
            }
            return o;
        };

        return flux::fold(seq, fold_fn, Opt{});
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto min = detail::min_op{};
FLUX_EXPORT inline constexpr auto max = detail::max_op{};
FLUX_EXPORT inline constexpr auto minmax = detail::minmax_op{};

template <typename Derived>
template <typename Cmp>
    requires weak_ordering_for<Cmp, Derived>
constexpr auto inline_iter_base<Derived>::max(Cmp cmp)
{
    return flux::max(derived(), std::move(cmp));
}

template <typename Derived>
template <typename Cmp>
    requires weak_ordering_for<Cmp, Derived>
constexpr auto inline_iter_base<Derived>::min(Cmp cmp)
{
    return flux::min(derived(), std::move(cmp));
}

template <typename Derived>
template <typename Cmp>
    requires weak_ordering_for<Cmp, Derived>
constexpr auto inline_iter_base<Derived>::minmax(Cmp cmp)
{
    return flux::minmax(derived(), std::move(cmp));
}

} // namespace flux

#endif // FLUX_ALGORITHM_MINMAX_HPP_INCLUDED
