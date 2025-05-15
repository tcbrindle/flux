
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_STARTS_WITH_HPP_INCLUDED
#define FLUX_ALGORITHM_STARTS_WITH_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

FLUX_EXPORT
struct starts_with_t {
    template <iterable Haystack, iterable Needle, typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, element_t<Haystack>, element_t<Needle>>
    [[nodiscard]]
    constexpr auto operator()(Haystack&& haystack, Needle&& needle, Cmp cmp = Cmp{}) const -> bool
    {
        if constexpr (sized_iterable<Haystack> && sized_iterable<Needle>) {
            if (flux::iterable_size(haystack) < flux::iterable_size(needle)) {
                return false;
            }
        }

        iteration_context auto haystack_ctx = iterate(haystack);
        iteration_context auto needle_ctx = iterate(needle);

        while (true) {
            auto haystack_elem = next_element(haystack_ctx);
            auto needle_elem = next_element(needle_ctx);

            if (haystack_elem.has_value() && needle_elem.has_value()) {
                if (!std::invoke(cmp, haystack_elem.value(), needle_elem.value())) {
                    return false;
                }
            } else if (needle_elem.has_value()) {
                return false;
            } else {
                return true;
            }
        }
    }
};

FLUX_EXPORT inline constexpr starts_with_t starts_with{};

template <typename Derived>
template <sequence Needle, typename Cmp>
    requires std::predicate<Cmp&, element_t<Derived>, element_t<Needle>>
constexpr auto inline_sequence_base<Derived>::starts_with(Needle&& needle, Cmp cmp) -> bool
{
    return flux::starts_with(derived(), FLUX_FWD(needle), std::move(cmp));
}


} // namespace flux

#endif // FLUX_ALGORITHM_STARTS_WITH_HPP_INCLUDED
