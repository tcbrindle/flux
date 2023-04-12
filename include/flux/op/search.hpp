
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SEARCH_HPP_INCLUDED
#define FLUX_OP_SEARCH_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

struct search_fn {
    template <multipass_sequence Haystack, multipass_sequence Needle,
              typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, element_t<Haystack>, element_t<Needle>>
    constexpr auto operator()(Haystack&& h, Needle&& n, Cmp cmp = {}) const
        -> bounds_t<Haystack>
    {
        auto hfirst = flux::first(h);

        while(true) {
            auto cur1 = hfirst;
            auto cur2 = flux::first(n);

            while (true) {
                if (is_last(n, cur2)) {
                    return {std::move(hfirst), std::move(cur1)};
                }

                if (is_last(h, cur1)) {
                    return {cur1, cur1};
                }

                if (!std::invoke(cmp, read_at(h, cur1), read_at(n, cur2))) {
                    break;
                }

                inc(h, cur1);
                inc(n, cur2);
            }

            inc(h, hfirst);
        }
    }

};

} // namespace detail

inline constexpr auto search = detail::search_fn{};

} // namespace flux

#endif

