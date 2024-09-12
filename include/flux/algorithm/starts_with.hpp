
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_STARTS_WITH_HPP_INCLUDED
#define FLUX_ALGORITHM_STARTS_WITH_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

struct starts_with_fn {
    template <sequence Haystack, sequence Needle, typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, element_t<Haystack>, element_t<Needle>>
    constexpr auto operator()(Haystack&& haystack, Needle&& needle, Cmp cmp = Cmp{}) const -> bool
    {
        if constexpr (sized_sequence<Haystack> && sized_sequence<Needle>) {
            if (flux::size(haystack) < flux::size(needle)) {
                return false;
            }
        }

        auto n_cur = flux::first(needle);
        if (flux::is_last(needle, n_cur)) {
            return true; // trivially start with an empty sequence
        }
        bool matched = false;

        bool haystack_completed = iterate(haystack, [&](auto&& h_elem) -> bool {
            if (flux::is_last(needle, n_cur)) {
                matched = true;
                return false; // break;
            }

            if (!std::invoke(cmp, FLUX_FWD(h_elem), flux::read_at(needle, n_cur))) {
                flux::inc(needle, n_cur);
                return false; // break
            }

            flux::inc(needle, n_cur);

            return true;
        });

        if (haystack_completed && flux::is_last(needle, n_cur)) {
            matched = true;
        }

        return matched;
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto starts_with = detail::starts_with_fn{};

template <typename Derived>
template <sequence Needle, typename Cmp>
    requires std::predicate<Cmp&, element_t<Derived>, element_t<Needle>>
constexpr auto inline_sequence_base<Derived>::starts_with(Needle&& needle, Cmp cmp) -> bool
{
    return flux::starts_with(derived(), FLUX_FWD(needle), std::move(cmp));
}


} // namespace flux

#endif // FLUX_ALGORITHM_STARTS_WITH_HPP_INCLUDED
