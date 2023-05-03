
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ENDS_WITH_HPP_INCLUDED
#define FLUX_OP_ENDS_WITH_HPP_INCLUDED

#include <flux/op/count.hpp>
#include <flux/op/equal.hpp>

namespace flux {

namespace detail {

struct ends_with_fn {
private:
    template <typename H, typename N>
    static constexpr auto bidir_impl(H& h, N& n, auto& cmp) -> bool
    {
        if constexpr (sized_sequence<H> && sized_sequence<N>) {
            if (flux::size(h) < flux::size(n)) {
                return false;
            }
        }

        auto cur1 = flux::last(h);
        auto cur2 = flux::last(n);

        auto const f1 = flux::first(h);
        auto const f2 = flux::first(n);

        if (cur2 == f2) {
            return true;
        } else if (cur1 == f1) {
            return false;
        }

        while (true) {
            flux::dec(h, cur1);
            flux::dec(n, cur2);

            if (!std::invoke(cmp, flux::read_at(h, cur1), flux::read_at(n, cur2))) {
                return false;
            }

            if (cur2 == f2) {
                return true;
            } else if (cur1 == f1) {
                return false;
            }
        }
    }

public:
    template <sequence Haystack, sequence Needle, typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, element_t<Haystack>, element_t<Needle>> &&
                 (multipass_sequence<Haystack> || sized_sequence<Haystack>) &&
                 (multipass_sequence<Needle> || sized_sequence<Needle>)
    constexpr auto operator()(Haystack&& haystack, Needle&& needle, Cmp cmp = Cmp{}) const
        -> bool
    {
        if constexpr(bidirectional_sequence<Haystack> &&
                     bounded_sequence<Haystack> &&
                     bidirectional_sequence<Needle> &&
                     bounded_sequence<Needle>) {
            return bidir_impl(haystack, needle, cmp);
        } else {
            distance_t len1 = flux::count(haystack);
            distance_t len2 = flux::count(needle);

            if (len1 < len2) {
                return false;
            }

            auto cur1 = flux::first(haystack);
            detail::advance(haystack, cur1, len1 - len2);

            return flux::equal(flux::slice(haystack, std::move(cur1), flux::last),
                               needle, std::move(cmp));
        }
    }
};

} // namespace detail

inline constexpr auto ends_with = detail::ends_with_fn{};

template <typename Derived>
template <sequence Needle, typename Cmp>
    requires std::predicate<Cmp&, element_t<Derived>, element_t<Needle>> &&
             (multipass_sequence<Derived> || sized_sequence<Derived>) &&
             (multipass_sequence<Needle> || sized_sequence<Needle>)
constexpr auto inline_sequence_base<Derived>::ends_with(Needle&& needle, Cmp cmp) -> bool
{
    return flux::ends_with(derived(), FLUX_FWD(needle), std::move(cmp));
}


} // namespace flux

#endif // FLUX_OP_ENDS_WITH_HPP_INCLUDED
