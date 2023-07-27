
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SWAP_ELEMENTS_HPP_INCLUDED
#define FLUX_OP_SWAP_ELEMENTS_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

struct swap_elements_fn {
    template <sequence Seq1, sequence Seq2>
        requires element_swappable_with<Seq1&, Seq2&>
    constexpr void operator()(Seq1&& seq1, Seq2&& seq2) const
    {
        auto cur1 = flux::first(seq1);
        auto cur2 = flux::first(seq2);

        while (!flux::is_last(seq1, cur1) && !flux::is_last(seq2, cur2)) {
            flux::swap_with(seq1, cur1, seq2, cur2);
            flux::inc(seq1, cur1);
            flux::inc(seq2, cur2);
        }
    }
};

}

FLUX_EXPORT inline constexpr auto swap_elements = detail::swap_elements_fn{};

}

#endif
