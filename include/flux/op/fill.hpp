// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FILL_HPP_INCLUDED
#define FLUX_OP_FILL_HPP_INCLUDED

#include <flux/op/for_each.hpp>

namespace flux {

namespace detail {

struct fill_fn {
    template <typename Value, writable_sequence_of<Value> Seq>
    constexpr void operator()(Seq&& seq, Value const& value) const
    {
        flux::for_each(seq, [&value](auto&& elem) {
            FLUX_FWD(elem) = value;
        });
    }
};

} // namespace detail

inline constexpr auto fill = detail::fill_fn{};

template <typename D>
template <typename Value>
    requires writable_sequence_of<D, Value const&>
constexpr void lens_base<D>::fill(Value const& value)
{
    flux::fill(derived(), value);
}

} // namespace flux

#endif
