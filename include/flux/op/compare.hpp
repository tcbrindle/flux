
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_COMPARE_HPP_INCLUDED
#define FLUX_OP_COMPARE_HPP_INCLUDED

#include <flux/core.hpp>

#include <compare>
#include <cstring>
#include <bit>

namespace flux {

namespace detail {

template <typename Cmp>
concept is_comparison_category =
    std::same_as<Cmp, std::strong_ordering> ||
    std::same_as<Cmp, std::weak_ordering> ||
    std::same_as<Cmp, std::partial_ordering>;

struct compare_fn {
private:
    template <typename Seq1, typename Seq2, typename Cmp>
    static constexpr auto impl(Seq1& seq1, Seq2& seq2, Cmp& cmp)
        -> std::decay_t<
            std::invoke_result_t<Cmp &, element_t<Seq1>, element_t<Seq2>>>
    {
        auto cur1 = flux::first(seq1);
        auto cur2 = flux::first(seq2);

        while (!flux::is_last(seq1, cur1) && !flux::is_last(seq2, cur2)) {
            if (auto r = std::invoke(cmp, flux::read_at(seq1, cur1), flux::read_at(seq2, cur2));
                r != 0) {
                return r;
            }
            flux::inc(seq1, cur1);
            flux::inc(seq2, cur2);
        }

        return !flux::is_last(seq1, cur1) ? std::strong_ordering::greater :
               !flux::is_last(seq2, cur2) ? std::strong_ordering::less :
                                            std::strong_ordering::equal;
    }

public:
    template <sequence Seq1, sequence Seq2,
        typename Cmp = std::compare_three_way>
        requires std::invocable<Cmp &, element_t<Seq1>, element_t<Seq2>> &&
        is_comparison_category<std::decay_t<
            std::invoke_result_t<Cmp &, element_t<Seq1>, element_t<Seq2>>>>
    constexpr auto operator()(Seq1 &&seq1, Seq2 &&seq2, Cmp cmp = {}) const
        -> std::decay_t<
            std::invoke_result_t<Cmp &, element_t<Seq1>, element_t<Seq2>>>
    {
        constexpr bool can_memcmp = 
            std::same_as<Cmp, std::compare_three_way> &&
            contiguous_sequence<Seq1> && 
            contiguous_sequence<Seq2> &&
            sized_sequence<Seq1> && 
            sized_sequence<Seq2> &&
            std::same_as<value_t<Seq1>, value_t<Seq2>> &&
            std::unsigned_integral<value_t<Seq1>> &&
            ((sizeof(value_t<Seq1>) == 1) || (std::endian::native == std::endian::big));

        if constexpr (can_memcmp) {
            if (std::is_constant_evaluated()) {
                return impl(seq1, seq2, cmp);
            } else {
                auto const seq1_size = flux::usize(seq1);
                auto const seq2_size = flux::usize(seq2);
                auto min_size = std::min(seq1_size, seq2_size);

                int cmp_result = 0;
                if(min_size > 0)
                {
                    auto data1 = flux::data(seq1);
                    FLUX_ASSERT(data1 != nullptr);
                    auto data2 = flux::data(seq2);
                    FLUX_ASSERT(data2 != nullptr);

                    cmp_result = std::memcmp(data1, data2, min_size);
                }

                if (cmp_result == 0) {
                    if (seq1_size == seq2_size) {
                        return std::strong_ordering::equal;
                    } else if (seq1_size < seq2_size) {
                        return std::strong_ordering::less;
                    } else /* seq1_size > seq2_size */ {
                        return std::strong_ordering::greater;
                    }
                } else if (cmp_result > 0) {
                    return std::strong_ordering::greater;
                } else /* cmp_result < 0 */ {
                    return std::strong_ordering::less;
                }
            }
        } else {
            return impl(seq1, seq2, cmp);
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto compare = detail::compare_fn{};

} // namespace flux

#endif // FLUX_OP_EQUAL_HPP_INCLUDED
