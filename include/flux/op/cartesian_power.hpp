
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_POWER_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_POWER_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/core/numeric.hpp>
#include <flux/op/from.hpp>
#include <flux/op/cartesian_base.hpp>

#include <tuple>

namespace flux {

namespace detail {

template <std::size_t PowN, sequence Base>
struct cartesian_power_adaptor
    : inline_sequence_base<cartesian_power_adaptor<PowN, Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;

public:
    constexpr explicit cartesian_power_adaptor(Base&& base)
        : base_(FLUX_FWD(base))
    {}

    using flux_sequence_traits = cartesian_traits_base<
        PowN,
        cartesian_kind::power,
        read_kind::tuple,
        Base
    >;
    friend flux_sequence_traits;
};



template<std::size_t PowN>
struct cartesian_power_fn {

    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        return cartesian_power_adaptor<PowN, std::decay_t<Seq>>(
                    FLUX_FWD(seq));
    }
};


} // end namespace detail

template<std::size_t PowN>
FLUX_EXPORT inline constexpr auto cartesian_power = detail::cartesian_power_fn<PowN>{};

} // end namespace flux

#endif

