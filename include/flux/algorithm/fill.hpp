// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_FILL_HPP_INCLUDED
#define FLUX_ALGORITHM_FILL_HPP_INCLUDED

#include <flux/algorithm/for_each.hpp>
#include <type_traits>
#include <cstring>

namespace flux {

FLUX_EXPORT
struct fill_t {
private:
    template <typename It, typename Value>
    static constexpr auto impl(It&& it, Value const& value) -> void
    {
        flux::for_each(it, [&value](auto&& elem) { FLUX_FWD(elem) = value; });
    }

    template <typename Seq, typename Value>
    static constexpr auto memset_impl(Seq& seq, Value const& value) -> void
    {
        if (std::is_constant_evaluated()) {
            impl(seq, value); // LCOV_EXCL_LINE
        } else {
            auto size = flux::usize(seq);
            if (size == 0) {
                return;
            }

            FLUX_ASSERT(flux::data(seq) != nullptr);

            std::memset(flux::data(seq), value, size * sizeof(value_t<Seq>));
        }
    }

public:
    template <iterable It, typename Value>
        requires std::assignable_from<element_t<It>, Value const&>
    constexpr auto operator()(It&& it, Value const& value) const -> void
    {
        if constexpr (contiguous_sequence<It> && sized_sequence<It>
                      && std::same_as<Value, iterable_value_t<It>>
                      // only allow memset on single byte types
                      && sizeof(value_t<It>) == 1 && std::is_trivially_copyable_v<value_t<It>>) {
            memset_impl(it, value);
        } else {
            impl(it, value);
        }
    }
};

FLUX_EXPORT inline constexpr fill_t fill {};

template <typename D>
template <typename Value>
    requires writable_sequence_of<D, Value const&>
constexpr void inline_sequence_base<D>::fill(Value const& value)
{
    flux::fill(derived(), value);
}

} // namespace flux

#endif // FLUX_ALGORITHM_FILL_HPP_INCLUDED
