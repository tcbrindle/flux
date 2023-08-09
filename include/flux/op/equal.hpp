
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_EQUAL_HPP_INCLUDED
#define FLUX_OP_EQUAL_HPP_INCLUDED

#include <flux/core.hpp>
#include <type_traits>
#include <cstring>

namespace flux {

namespace detail {

struct equal_fn {
private:
    template <typename Seq1, typename Seq2, typename Cmp>
    static constexpr auto impl(Seq1& seq1, Seq2& seq2, Cmp cmp)
    {
        auto cur1 = flux::first(seq1);
        auto cur2 = flux::first(seq2);

        while (!flux::is_last(seq1, cur1) && !flux::is_last(seq2, cur2)) {
            if (!std::invoke(cmp, flux::read_at(seq1, cur1), flux::read_at(seq2, cur2))) {
                return false;
            }
            flux::inc(seq1, cur1);
            flux::inc(seq2, cur2);
        }

        return flux::is_last(seq1, cur1) == flux::is_last(seq2, cur2);
    }

public:
    template <sequence Seq1, sequence Seq2, typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, element_t<Seq1>, element_t<Seq2>>
    constexpr auto operator()(Seq1&& seq1, Seq2&& seq2, Cmp cmp = {}) const
        -> bool
    {
        if constexpr (sized_sequence<Seq1> && sized_sequence<Seq2>) {
            if (flux::size(seq1) != flux::size(seq2)) {
                return false;
            }
        }

        constexpr bool can_memcmp = contiguous_sequence<Seq1> &&
            contiguous_sequence<Seq2> && sized_sequence<Seq1> &&
            sized_sequence<Seq2> &&
            std::is_trivially_copyable_v<value_t<Seq1>> &&
            std::is_trivially_copyable_v<value_t<Seq2>> &&
            std::same_as<value_t<Seq1>, value_t<Seq2>>;

        if constexpr (can_memcmp) {
            if (std::is_constant_evaluated()) {
                return impl(seq1, seq2, cmp);
            } else {
                auto result = std::memcmp(flux::data(seq1), flux::data(seq2),
                    flux::usize(seq1) * sizeof(value_t<Seq1>));
                return result == 0;
            }
        } else {
            return impl(seq1, seq2, cmp);
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto equal = detail::equal_fn{};

} // namespace flux

#endif // FLUX_OP_EQUAL_HPP_INCLUDED
