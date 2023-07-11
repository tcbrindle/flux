
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_PRODUCT_WITH_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_PRODUCT_WITH_HPP_INCLUDED

#include <flux/op/cartesian_product.hpp>

namespace flux {

namespace detail {

template <typename Func, sequence... Bases>
struct cartesian_product_with_adaptor
    : inline_sequence_base<cartesian_product_with_adaptor<Func, Bases...>> {
private:
    FLUX_NO_UNIQUE_ADDRESS std::tuple<Bases...> bases_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

    friend struct cartesian_product_traits_base<Bases...>;
    friend struct sequence_traits<cartesian_product_with_adaptor>;

public:
    constexpr explicit cartesian_product_with_adaptor(decays_to<Func> auto&& func, decays_to<Bases> auto&&... bases)
        : bases_(FLUX_FWD(bases)...),
          func_(FLUX_FWD(func))
    {}

    struct flux_sequence_traits : detail::cartesian_product_traits_base<Bases...>
    {
        template <typename Self>
        static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
            -> decltype(auto)
        {
            return [&]<std::size_t... N>(std::index_sequence<N...>) -> decltype(auto) {
                return std::invoke(self.func_, flux::read_at(std::get<N>(self.bases_), std::get<N>(cur))...);
            }(std::index_sequence_for<Bases...>{});
        }
    };
};

struct cartesian_product_with_fn
{
    template <typename Func, adaptable_sequence Seq0, adaptable_sequence... Seqs>
        requires (multipass_sequence<Seqs> && ...) &&
        std::regular_invocable<Func&, element_t<Seq0>, element_t<Seqs>...>
    [[nodiscard]]
    constexpr auto operator()(Func func, Seq0&& seq0, Seqs&&... seqs) const
    {
        return cartesian_product_with_adaptor<Func, std::decay_t<Seq0>, std::decay_t<Seqs>...>(
                    std::move(func), FLUX_FWD(seq0), FLUX_FWD(seqs)...);
    }
};



} // namespace detail



inline constexpr auto cartesian_product_with = detail::cartesian_product_with_fn{};

} // namespace flux

#endif
