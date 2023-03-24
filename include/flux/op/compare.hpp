
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_COMPARE_HPP_INCLUDED
#define FLUX_OP_COMPARE_HPP_INCLUDED

#include <flux/core.hpp>

#include <compare>

namespace flux {

namespace detail {

template <typename Cmp>
concept is_comparison_category =
    std::same_as<Cmp, std::strong_ordering> ||
    std::same_as<Cmp, std::weak_ordering> ||
    std::same_as<Cmp, std::partial_ordering>;

struct compare_fn {
    template <sequence Seq1, sequence Seq2, typename Cmp = std::compare_three_way,
              typename Proj1 = std::identity, typename Proj2 = std::identity>
        requires std::invocable<Cmp&, projected_t<Proj1, Seq1>, projected_t<Proj2, Seq2>> &&
                 is_comparison_category<std::decay_t<std::invoke_result_t<Cmp&, projected_t<Proj1, Seq1>, projected_t<Proj2, Seq2>>>>
    constexpr auto operator()(Seq1&& seq1, Seq2&& seq2, Cmp cmp = {},
                              Proj1 proj1 = {}, Proj2 proj2 = {}) const
        -> std::decay_t<std::invoke_result_t<Cmp&, projected_t<Proj1, Seq1>, projected_t<Proj2, Seq2>>>
    {
        auto cur1 = flux::first(seq1);
        auto cur2 = flux::first(seq2);

        while (!flux::is_last(seq1, cur1) && !flux::is_last(seq2, cur2)) {
            if (auto r = std::invoke(cmp, std::invoke(proj1, flux::read_at(seq1, cur1)),
                                          std::invoke(proj2, flux::read_at(seq2, cur2))); r != 0) {
                return r;
            }
            flux::inc(seq1, cur1);
            flux::inc(seq2, cur2);
        }

        return !flux::is_last(seq1, cur1) ? std::strong_ordering::greater :
               !flux::is_last(seq2, cur2) ? std::strong_ordering::less :
                                           std::strong_ordering::equal;
    }

};

} // namespace detail

inline constexpr auto compare = detail::compare_fn{};

} // namespace flux

#endif // FLUX_OP_EQUAL_HPP_INCLUDED
