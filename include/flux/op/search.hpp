
#pragma once

#include <flux/core.hpp>

namespace flux {

namespace detail {

struct search_fn {
    template <multipass_sequence Haystack, multipass_sequence Needle>
        // Requires...
    constexpr auto operator()(Haystack&& h, Needle&& n) const
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

                if (read_at(h, cur1) != read_at(n, cur2)) {
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
