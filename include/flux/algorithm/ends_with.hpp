
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_ENDS_WITH_HPP_INCLUDED
#define FLUX_ALGORITHM_ENDS_WITH_HPP_INCLUDED

#include <flux/adaptor/drop.hpp>
#include <flux/adaptor/reverse.hpp>
#include <flux/algorithm/count.hpp>
#include <flux/algorithm/equal.hpp>
#include <flux/algorithm/starts_with.hpp>

namespace flux {

FLUX_EXPORT
struct ends_with_t {
    template <iterable Haystack, iterable Needle, typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, iterable_element_t<Haystack>, iterable_element_t<Needle>>
        && (multipass_sequence<Haystack> || sized_iterable<Haystack>)
        && (multipass_sequence<Needle> || sized_iterable<Needle>)
    [[nodiscard]]
    constexpr auto operator()(Haystack&& haystack, Needle&& needle, Cmp cmp = Cmp{}) const -> bool
    {
        if constexpr (reverse_iterable<Haystack> && reverse_iterable<Needle>) {
            return starts_with(reverse(from_fwd_ref(haystack)), reverse(from_fwd_ref(needle)),
                               std::ref(cmp));
        } else {
            int_t haystack_size = flux::count(haystack);
            int_t needle_size = flux::count(needle);

            if (haystack_size < needle_size) {
                return false;
            }

            return equal(drop(from_fwd_ref(haystack), num::sub(haystack_size, needle_size)), needle,
                         std::ref(cmp));
        }
    }
};

FLUX_EXPORT inline constexpr auto ends_with = ends_with_t{};

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

#endif // FLUX_ALGORITHM_ENDS_WITH_HPP_INCLUDED
