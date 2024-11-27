
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_CARTESIAN_PRODUCT_MAP_HPP_INCLUDED
#define FLUX_ADAPTOR_CARTESIAN_PRODUCT_MAP_HPP_INCLUDED

#include <flux/adaptor/cartesian_base.hpp>

namespace flux {

namespace detail {

template <typename Func, sequence... Bases>
struct cartesian_product_map_adaptor
    : inline_iter_base<cartesian_product_map_adaptor<Func, Bases...>> {
private:
    FLUX_NO_UNIQUE_ADDRESS std::tuple<Bases...> bases_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

public:
    constexpr explicit cartesian_product_map_adaptor(decays_to<Func> auto&& func, decays_to<Bases> auto&&... bases)
        : bases_(FLUX_FWD(bases)...),
          func_(FLUX_FWD(func))
    {}

    using flux_iter_traits = cartesian_traits_base<
        sizeof...(Bases),
        cartesian_kind::product,
        read_kind::map,
        Bases...
    >;
    friend flux_iter_traits::impl;
};

struct cartesian_product_map_fn
{
    template <typename Func, adaptable_sequence Seq0, adaptable_sequence... Seqs>
        requires (multipass_sequence<Seqs> && ...) &&
        std::regular_invocable<Func&, element_t<Seq0>, element_t<Seqs>...>
    [[nodiscard]]
    constexpr auto operator()(Func func, Seq0&& seq0, Seqs&&... seqs) const
    {
        return cartesian_product_map_adaptor<Func, std::decay_t<Seq0>, std::decay_t<Seqs>...>(
                    std::move(func), FLUX_FWD(seq0), FLUX_FWD(seqs)...);
    }
};



} // namespace detail

FLUX_EXPORT inline constexpr auto cartesian_product_map = detail::cartesian_product_map_fn{};

} // namespace flux

#endif // FLUX_ADAPTOR_CARTESIAN_PRODUCT_MAP_HPP_INCLUDED
