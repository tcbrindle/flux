// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_FIND_ELEMENT_HPP_INCLUDED
#define FLUX_ALGORITHM_FIND_ELEMENT_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

FLUX_EXPORT
struct find_element_if_t {
    template <iterable It, typename Pred>
        requires std::predicate<Pred&, iterable_element_t<It>>
    constexpr auto operator()(It&& it, Pred pred) const
    {
        iterable auto filtered = filter(std::ref(it), std::ref(pred));
        iteration_context auto ctx = iterate(filtered);
        return next_element(ctx);
    }
};

FLUX_EXPORT inline constexpr find_element_if_t find_element_if{};

FLUX_EXPORT
struct find_element_t {
    template <iterable It, typename Value>
        requires std::equality_comparable_with<iterable_element_t<It>, Value const&>
    constexpr auto operator()(It&& it, Value const& value) const
    {
        return find_element_if(it, [&value](auto&& elem) { return FLUX_FWD(elem) == value; });
    }
};

FLUX_EXPORT inline constexpr find_element_t find_element{};

} // namespace flux

#endif // FLUX_ALGORITHM_FIND_ELEMENT_HPP_INCLUDED
