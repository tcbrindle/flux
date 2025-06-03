
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_COMPARE_HPP_INCLUDED
#define FLUX_ALGORITHM_COMPARE_HPP_INCLUDED

#include <flux/core.hpp>

#include <compare>
#include <cstring>
#include <bit>

namespace flux {

FLUX_EXPORT
struct compare_t {
private:
    template <iterable It1, iterable It2, typename Cmp>
        requires ordering_invocable<Cmp&, iterable_element_t<It1>, iterable_element_t<It2>>
    static constexpr auto impl(It1& it1, It2& it2, Cmp& cmp) -> std::decay_t<
        std::invoke_result_t<Cmp&, iterable_element_t<It1>, iterable_element_t<It2>>>
    {
        iteration_context auto ctx1 = iterate(it1);
        iteration_context auto ctx2 = iterate(it2);

        while (true) {
            auto opt1 = next_element(ctx1);
            auto opt2 = next_element(ctx2);

            if (opt1.has_value() && opt2.has_value()) {
                auto r = std::invoke(cmp, opt1.value(), opt2.value());
                if (r != 0) {
                    return r;
                }
            } else if (opt1.has_value()) {
                return std::strong_ordering::greater;
            } else if (opt2.has_value()) {
                return std::strong_ordering::less;
            } else {
                return std::strong_ordering::equal;
            }
        }
    }

    template <contiguous_sequence Seq1, contiguous_sequence Seq2>
    static constexpr auto memcmp_impl(Seq1& seq1, Seq2& seq2) -> std::strong_ordering
    {
        auto const seq1_size = flux::usize(seq1);
        auto const seq2_size = flux::usize(seq2);
        auto min_size = (cmp::min)(seq1_size, seq2_size);

        int cmp_result = 0;
        if (min_size > 0) {
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

    template <typename It1, typename It2, typename Cmp>
    static consteval auto can_memcmp() -> bool
    {
        return std::same_as<Cmp, std::compare_three_way> && contiguous_sequence<It1>
            && contiguous_sequence<It2> && sized_sequence<It1> && sized_sequence<It2>
            && std::same_as<iterable_value_t<It1>, iterable_value_t<It2>>
            && std::unsigned_integral<iterable_value_t<It1>>
            && ((sizeof(iterable_value_t<It1>) == 1) || (std::endian::native == std::endian::big));
    }

public:
    template <iterable It1, iterable It2, typename Cmp = std::compare_three_way>
        requires ordering_invocable<Cmp&, iterable_element_t<It1>, iterable_element_t<It2>>
    constexpr auto operator()(It1&& it1, It2&& it2, Cmp cmp = {}) const -> std::decay_t<
        std::invoke_result_t<Cmp&, iterable_element_t<It1>, iterable_element_t<It2>>>
    {
        if constexpr (can_memcmp<It1, It2, Cmp>()) {
            if (std::is_constant_evaluated()) {
                return impl(it1, it2, cmp); // LCOV_EXCL_LINE
            } else {
                return memcmp_impl(it1, it2);
            }
        } else {
            return impl(it1, it2, cmp);
        }
    }
};

FLUX_EXPORT inline constexpr auto compare = compare_t{};

} // namespace flux

#endif // FLUX_ALGORITHM_COMPARE_HPP_INCLUDED
