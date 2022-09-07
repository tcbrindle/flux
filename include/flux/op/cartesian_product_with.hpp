
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_PRODUCT_WITH_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_PRODUCT_WITH_HPP_INCLUDED

#include <flux/op/cartesian_product.hpp>

namespace flux {

namespace detail {

template <typename Func, lens... Bases>
struct cartesian_product_with_adaptor
    : lens_base<cartesian_product_with_adaptor<Func, Bases...>> {
private:
    FLUX_NO_UNIQUE_ADDRESS std::tuple<Bases...> bases_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

    friend struct cartesian_product_iface_base<Bases...>;
    friend struct sequence_iface<cartesian_product_with_adaptor>;

public:
    constexpr explicit cartesian_product_with_adaptor(Func func, Bases&&... bases)
        : bases_(std::move(bases)...),
          func_(std::move(func))
    {}
};

struct cartesian_product_with_fn
{
    template <typename Func, adaptable_sequence Seq0, adaptable_sequence... Seqs>
        requires (multipass_sequence<Seqs> && ...) &&
        std::regular_invocable<Func&, element_t<Seq0>, element_t<Seqs>...>
    constexpr auto operator()(Func func, Seq0&& seq0, Seqs&&... seqs) const
    {
        return cartesian_product_with_adaptor(std::move(func),
                                              flux::from(FLUX_FWD(seq0)),
                                              flux::from(FLUX_FWD(seqs))...);
    }
};



} // namespace detail

template <typename Func, typename... Bases>
struct sequence_iface<detail::cartesian_product_with_adaptor<Func, Bases...>>
    : detail::cartesian_product_iface_base<Bases...>
{
    //using detail::cartesian_product_iface_base<Bases...>::const_like_t;

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) {
            return std::invoke(self.func_, flux::read_at(std::get<N>(self.bases_), std::get<N>(cur))...);
        }(std::index_sequence_for<Bases...>{});
    }
};

inline constexpr auto cartesian_product_with = detail::cartesian_product_with_fn{};

} // namespace flux

#endif
