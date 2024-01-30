
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FIND_MIN_MAX_HPP_INCLUDED
#define FLUX_OP_FIND_MIN_MAX_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/minmax.hpp>

namespace flux {

namespace detail {

struct find_min_fn {
    template <multipass_sequence Seq,
              strict_weak_order_for<Seq> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Cmp cmp = {}) const -> cursor_t<Seq>
    {
        auto min = first(seq);
        if (!is_last(seq, min)) {
            for (auto cur = next(seq, min); !is_last(seq, cur); inc(seq, cur)) {
                if (std::invoke(cmp, read_at(seq, cur), read_at(seq, min)) < 0) {
                    min = cur;
                }
            }
        }

        return min;
    }
};

struct find_max_fn {
    template <multipass_sequence Seq,
              strict_weak_order_for<Seq> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Cmp cmp = {}) const -> cursor_t<Seq>
    {
        auto max = first(seq);
        if (!is_last(seq, max)) {
            for (auto cur = next(seq, max); !is_last(seq, cur); inc(seq, cur)) {
                if (!(std::invoke(cmp, read_at(seq, cur), read_at(seq, max)) < 0)) {
                    max = cur;
                }
            }
        }

        return max;
    }
};

struct find_minmax_fn {
    template <multipass_sequence Seq,
              strict_weak_order_for<Seq> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Cmp cmp = {}) const
        -> minmax_result<cursor_t<Seq>>
    {
        auto min = first(seq);
        auto max = min;
        if (!is_last(seq, min)) {
            for (auto cur = next(seq, min); !is_last(seq, cur); inc(seq, cur)) {
                auto&& elem = read_at(seq, cur);

                if (std::invoke(cmp, elem, read_at(seq, min)) < 0) {
                    min = cur;
                }
                if (!(std::invoke(cmp, elem, read_at(seq, max)) < 0)) {
                    max = cur;
                }
            }
        }

        return {std::move(min), std::move(max)};
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto find_min = detail::find_min_fn{};
FLUX_EXPORT inline constexpr auto find_max = detail::find_max_fn{};
FLUX_EXPORT inline constexpr auto find_minmax = detail::find_minmax_fn{};

template <typename D>
template <typename Cmp>
    requires strict_weak_order_for<Cmp, D>
constexpr auto inline_sequence_base<D>::find_min(Cmp cmp)
{
    return flux::find_min(derived(), std::move(cmp));
}

template <typename D>
template <typename Cmp>
    requires strict_weak_order_for<Cmp, D>
constexpr auto inline_sequence_base<D>::find_max(Cmp cmp)
{
    return flux::find_max(derived(), std::move(cmp));
}

template <typename D>
template <typename Cmp>
    requires strict_weak_order_for<Cmp, D>
constexpr auto inline_sequence_base<D>::find_minmax(Cmp cmp)
{
    return flux::find_minmax(derived(), std::move(cmp));
}

} // namespace flux

#endif // FLUX_OP_FIND_MIN_MAX_HPP_INCLUDED
