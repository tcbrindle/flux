
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_SWAP_ELEMENTS_HPP_INCLUDED
#define FLUX_ALGORITHM_SWAP_ELEMENTS_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

FLUX_EXPORT
struct swap_elements_t {
    template <iterable It1, iterable It2>
        requires std::swappable_with<iterable_element_t<It1>, iterable_element_t<It2>>
    constexpr auto operator()(It1&& it1, It2&& it2) const -> void
    {
        iteration_context auto ctx1 = iterate(it1);
        iteration_context auto ctx2 = iterate(it2);

        while (true) {
            auto opt1 = next_element(ctx1);
            auto opt2 = next_element(ctx2);

            if (opt1.has_value() && opt2.has_value()) {
                std::ranges::swap(*std::move(opt1), *std::move(opt2));
            } else {
                break;
            }
        }
    }
};

FLUX_EXPORT inline constexpr swap_elements_t swap_elements{};
}

#endif // FLUX_ALGORITHM_SWAP_ELEMENTS_HPP_INCLUDED
