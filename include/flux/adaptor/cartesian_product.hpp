
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_CARTESIAN_PRODUCT_HPP_INCLUDED
#define FLUX_ADAPTOR_CARTESIAN_PRODUCT_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/core/numeric.hpp>
#include <flux/adaptor/cartesian_base.hpp>

#include <tuple>

namespace flux {

namespace detail {

template <sequence... Bases>
struct cartesian_product_adaptor
    : inline_sequence_base<cartesian_product_adaptor<Bases...>> {
private:
    FLUX_NO_UNIQUE_ADDRESS std::tuple<Bases...> bases_;

public:
    constexpr explicit cartesian_product_adaptor(decays_to<Bases> auto&&... bases)
        : bases_(FLUX_FWD(bases)...)
    {}
    
    using flux_sequence_traits = cartesian_traits_base<
        sizeof...(Bases),
        cartesian_kind::product,
        read_kind::tuple,
        Bases...
    >;
    friend flux_sequence_traits::impl;
};

struct cartesian_product_fn {
    template <adaptable_sequence Seq0, adaptable_sequence... Seqs>
        requires (multipass_sequence<Seqs> && ...)
    [[nodiscard]]
    constexpr auto operator()(Seq0&& seq0, Seqs&&... seqs) const
    {
        return cartesian_product_adaptor<std::decay_t<Seq0>, std::decay_t<Seqs>...>(
                    FLUX_FWD(seq0), FLUX_FWD(seqs)...);
    }
};

} // end namespace detail

FLUX_EXPORT inline constexpr auto cartesian_product = detail::cartesian_product_fn{};


} // end namespace flux

#endif // FLUX_ADAPTOR_CARTESIAN_PRODUCT_HPP_INCLUDED
