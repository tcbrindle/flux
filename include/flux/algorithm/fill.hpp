// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_FILL_HPP_INCLUDED
#define FLUX_ALGORITHM_FILL_HPP_INCLUDED

#include <flux/algorithm/for_each.hpp>
#include <type_traits>
#include <cstring>

namespace flux {

namespace detail {

struct fill_fn {
private:
    template <typename Seq, typename Value>
    static constexpr auto impl(Seq& seq, Value const& value)
    {
        flux::for_each(seq, [&value](auto&& elem) { FLUX_FWD(elem) = value; });
    }

public:
    template <typename Value, writable_sequence_of<Value> Seq>
    constexpr void operator()(Seq&& seq, Value const& value) const
    {
        constexpr bool can_memset = 
            contiguous_sequence<Seq> &&
            sized_sequence<Seq> &&
            std::same_as<Value, value_t<Seq>> &&
            // only allow memset on single byte types
            sizeof(value_t<Seq>) == 1 &&
            std::is_trivially_copyable_v<value_t<Seq>>;

        if constexpr (can_memset) {
            if (std::is_constant_evaluated()) {
                impl(seq, value); // LCOV_EXCL_LINE
            } else {
                auto size = flux::usize(seq);
                if(size == 0) {
                    return;
                }
                
                FLUX_ASSERT(flux::data(seq) != nullptr);
                
                std::memset(flux::data(seq), value,
                    size * sizeof(value_t<Seq>));
            }
        } else {
            impl(seq, value);
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto fill = detail::fill_fn{};

template <typename D>
template <typename Value>
    requires writable_sequence_of<D, Value const&>
constexpr void inline_sequence_base<D>::fill(Value const& value)
{
    flux::fill(derived(), value);
}

} // namespace flux

#endif // FLUX_ALGORITHM_FILL_HPP_INCLUDED
