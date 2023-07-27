
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_STARTS_WITH_HPP_INCLUDED
#define FLUX_OP_STARTS_WITH_HPP_INCLUDED

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

        auto h = flux::first(haystack);
        auto n = flux::first(needle);

        while (!flux::is_last(haystack, h) && !flux::is_last(needle, n)) {
            if (!std::invoke(cmp, flux::read_at(haystack, h), flux::read_at(needle, n))) {
                return false;
            }
            flux::inc(haystack, h);
            flux::inc(needle, n);
        }

        return flux::is_last(needle, n);
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

#endif // FLUX_OP_STARTS_WITH_HPP_INCLUDED
