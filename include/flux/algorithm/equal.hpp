
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_EQUAL_HPP_INCLUDED
#define FLUX_ALGORITHM_EQUAL_HPP_INCLUDED

#include <flux/core.hpp>
#include <type_traits>
#include <cstring>

namespace flux {

FLUX_EXPORT
struct equal_t {
private:
    template <typename It1, typename It2, typename Cmp>
    static constexpr auto impl(It1& it1, It2& it2, Cmp& cmp) -> bool
    {
        iteration_context auto ctx1 = iterate(it1);
        iteration_context auto ctx2 = iterate(it2);

        while (true) {
            flux::optional opt1 = next_element(ctx1);
            flux::optional opt2 = next_element(ctx2);

            if (opt1.has_value() && opt2.has_value()) {
                if (!std::invoke(cmp, opt1.value(), opt2.value())) {
                    return false;
                }
            } else if (opt1.has_value() || opt2.has_value()) {
                return false;
            } else {
                return true;
            }
        }
    }

    template <contiguous_sequence Seq1, contiguous_sequence Seq2>
    static constexpr auto memcmp_impl(Seq1& seq1, Seq2& seq2) -> bool
    {
        auto size = flux::usize(seq1);
        if (size == 0) {
            return true;
        }

        auto data1 = flux::data(seq1);
        auto data2 = flux::data(seq2);
        FLUX_ASSERT(data1 != nullptr);
        FLUX_ASSERT(data2 != nullptr);

        auto result = std::memcmp(data1, data2, size * sizeof(iterable_value_t<Seq1>));
        return result == 0;
    }

    template <typename It1, typename It2, typename Cmp>
    static consteval auto can_memcmp() -> bool
    {
        return std::same_as<Cmp, std::ranges::equal_to> && contiguous_sequence<It1>
            && contiguous_sequence<It2> && sized_sequence<It1> && sized_sequence<It2>
            && std::same_as<iterable_value_t<It1>, iterable_value_t<It2>>
            && (std::integral<iterable_value_t<It1>> || std::is_pointer_v<iterable_value_t<It1>>)
            && std::has_unique_object_representations_v<iterable_value_t<It1>>;
    }

public:
    template <iterable It1, iterable It2, typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, iterable_element_t<It1>, iterable_element_t<It2>>
    constexpr auto operator()(It1&& it1, It2&& it2, Cmp cmp = {}) const -> bool
    {
        if constexpr (sized_iterable<It1> && sized_iterable<It2>) {
            if (flux::iterable_size(it1) != flux::iterable_size(it2)) {
                return false;
            }
        }

        if constexpr (can_memcmp<It1, It2, Cmp>()) {
            if (std::is_constant_evaluated()) {
                return impl(it1, it2, cmp); // LCOV_EXCL_LINE
            } else {
                return memcmp_impl(it1, it2);
            }
        } else {
            return impl(it1, it2, cmp);
        }
    }

    template <iterable It1, iterable It2>
        requires iterable<iterable_element_t<It1>> && iterable<iterable_element_t<It2>>
        && (!std::equality_comparable_with<iterable_element_t<It1>, iterable_element_t<It2>>
            && std::is_invocable_v<equal_t&, It1&, It2&, equal_t&>)
    constexpr auto operator()(It1&& it1, It2&& it2) const -> bool
    {
        if constexpr (sized_iterable<It1> && sized_iterable<It2>) {
            if (flux::iterable_size(it1) != flux::iterable_size(it2)) {
                return false;
            }
        }

        return (*this)(it1, it2, *this);
    }
};

FLUX_EXPORT inline constexpr equal_t equal{};

} // namespace flux

#endif // FLUX_ALGORITHM_EQUAL_HPP_INCLUDED
