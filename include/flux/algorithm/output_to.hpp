
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_OUTPUT_TO_HPP_INCLUDED
#define FLUX_ALGORITHM_OUTPUT_TO_HPP_INCLUDED

#include <flux/algorithm/for_each.hpp>

#include <cstring>
#include <iterator>

namespace flux {

namespace detail {

struct output_to_fn {
private:
    template <typename Seq, typename Iter>
    static constexpr auto impl(Seq& seq, Iter& iter) -> Iter
    {
        flux::for_each(seq, [&iter](auto&& elem) {
            *iter = FLUX_FWD(elem);
            ++iter;
        });
        return iter;
    }

public:
    template <sequence Seq, std::weakly_incrementable Iter>
        requires std::indirectly_writable<Iter, element_t<Seq>>
    constexpr auto operator()(Seq&& seq, Iter iter) const -> Iter
    {
        constexpr bool can_memcpy =
            contiguous_sequence<Seq> &&
            sized_sequence<Seq> &&
            std::contiguous_iterator<Iter> &&
            std::is_trivially_copyable_v<value_t<Seq>>;

        if constexpr (can_memcpy) {
            if (std::is_constant_evaluated()) {
                return impl(seq, iter); // LCOV_EXCL_LINE
            } else {
                auto size = flux::usize(seq);
                if (size == 0) {
                    return iter;
                }
                FLUX_ASSERT(flux::data(seq) != nullptr);
                std::memmove(std::to_address(iter), flux::data(seq),
                             size * sizeof(value_t<Seq>));
                return iter + num::checked_cast<std::iter_difference_t<Iter>>(flux::size(seq));
            }
        } else {
            return impl(seq, iter);
        }
    }
};

}

FLUX_EXPORT inline constexpr auto output_to = detail::output_to_fn{};

template <typename D>
template <typename Iter>
    requires std::weakly_incrementable<Iter> &&
             std::indirectly_writable<Iter, element_t<D>>
constexpr auto inline_sequence_base<D>::output_to(Iter iter) -> Iter
{
    return flux::output_to(derived(), std::move(iter));
}

}

#endif // FLUX_ALGORITHM_OUTPUT_TO_HPP_INCLUDED
