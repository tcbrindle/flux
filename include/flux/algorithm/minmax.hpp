
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

FLUX_EXPORT
struct min_t {
    template <iterable It, weak_ordering_for<It> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Cmp cmp = Cmp{}) const
        -> flux::optional<iterable_value_t<It>>
    {
        return fold_first(FLUX_FWD(it), [&](auto min, auto&& elem) -> iterable_value_t<It> {
            if (std::invoke(cmp, elem, min) < 0) {
                return iterable_value_t<It>(FLUX_FWD(elem));
            } else {
                return min;
            }
        });
    }
};

FLUX_EXPORT inline constexpr min_t min{};

FLUX_EXPORT
struct max_t {
    template <iterable It, weak_ordering_for<It> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Cmp cmp = Cmp{}) const
        -> flux::optional<iterable_value_t<It>>
    {
        return fold_first(FLUX_FWD(it), [&](auto max, auto&& elem) -> iterable_value_t<It> {
            if (!(std::invoke(cmp, elem, max) < 0)) {
                return iterable_value_t<It>(FLUX_FWD(elem));
            } else {
                return max;
            }
        });
    }
};

FLUX_EXPORT inline constexpr max_t max{};

FLUX_EXPORT
struct minmax_t {
    template <iterable It, weak_ordering_for<It> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Cmp cmp = Cmp{}) const
        -> flux::optional<minmax_result<iterable_value_t<It>>>
    {
        using R = minmax_result<iterable_value_t<It>>;

        iteration_context auto ctx = iterate(it);

        auto opt = next_element(ctx);
        if (!opt.has_value()) {
            return flux::nullopt;
        }

        auto min = iterable_value_t<It>(opt.value());
        auto max = iterable_value_t<It>(std::move(opt).value());

        run_while(ctx, [&](auto&& elem) {
            if (std::invoke(cmp, elem, min) < 0) {
                min = iterable_value_t<It>(elem);
            }
            if (!(std::invoke(cmp, elem, max) < 0)) {
                max = iterable_value_t<It>(FLUX_FWD(elem));
            }
            return loop_continue;
        });

        return flux::optional<R>(R(std::move(min), std::move(max)));
    }
};

FLUX_EXPORT inline constexpr minmax_t minmax{};

template <typename Derived>
template <typename Cmp>
    requires weak_ordering_for<Cmp, Derived>
constexpr auto inline_sequence_base<Derived>::max(Cmp cmp)
{
    return flux::max(derived(), std::move(cmp));
}

template <typename Derived>
template <typename Cmp>
    requires weak_ordering_for<Cmp, Derived>
constexpr auto inline_sequence_base<Derived>::min(Cmp cmp)
{
    return flux::min(derived(), std::move(cmp));
}

template <typename Derived>
template <typename Cmp>
    requires weak_ordering_for<Cmp, Derived>
constexpr auto inline_sequence_base<Derived>::minmax(Cmp cmp)
{
    return flux::minmax(derived(), std::move(cmp));
}

} // namespace flux

#endif // FLUX_ALGORITHM_MINMAX_HPP_INCLUDED
