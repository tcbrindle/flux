
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
            auto idx1 = hfirst;
            auto idx2 = flux::first(n);

            while (true) {
                if (is_last(n, idx2)) {
                    return {std::move(hfirst), std::move(idx1)};
                }

                if (is_last(h, idx1)) {
                    return {idx1, idx1};
                }

                if (read_at(h, idx1) != read_at(n, idx2)) {
                    break;
                }

                inc(h, idx1);
                inc(n, idx2);
            }

            inc(h, hfirst);
        }
    }

};

} // namespace detail

inline constexpr auto search = detail::search_fn{};

} // namespace flux
