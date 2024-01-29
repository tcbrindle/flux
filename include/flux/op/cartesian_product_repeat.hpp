
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_PRODUCT_REPEAT_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_PRODUCT_REPEAT_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/core/numeric.hpp>
#include <flux/op/from.hpp>
#include <flux/op/cartesian_base.hpp>

#include <tuple>

namespace flux {

namespace detail {

template <std::size_t PowN, sequence Base>
struct cartesian_product_repeat_adaptor
    : inline_sequence_base<cartesian_product_repeat_adaptor<PowN, Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;

public:
    constexpr explicit cartesian_product_repeat_adaptor(Base&& base)
        : base_(FLUX_FWD(base))
    {}

    using flux_sequence_traits = cartesian_traits_base<
        PowN,
        cartesian_kind::power,
        read_kind::tuple,
        Base
    >;
    friend flux_sequence_traits::impl;
};



template<std::size_t PowN>
struct cartesian_product_repeat_fn {

    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr(PowN == 0) {
            return empty<std::tuple<>>;
        } else {
            return cartesian_product_repeat_adaptor<PowN, std::decay_t<Seq>>(
                FLUX_FWD(seq));
        }
    }
};


} // end namespace detail

FLUX_EXPORT
template<std::size_t PowN>
inline constexpr auto cartesian_product_repeat = detail::cartesian_product_repeat_fn<PowN>{};

} // end namespace flux

#endif

