
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_POWER_MAP_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_POWER_MAP_HPP_INCLUDED

#include <flux/op/cartesian_base.hpp>
#include <flux/op/requirements.hpp>

namespace flux {

namespace detail {

template <std::size_t PowN, typename Func, sequence Base>
struct cartesian_power_map_adaptor
    : inline_sequence_base<cartesian_power_map_adaptor<PowN, Func, Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

public:
    constexpr explicit cartesian_power_map_adaptor(decays_to<Func> auto&& func, decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base)),
          func_(FLUX_FWD(func))
    {}

    using flux_sequence_traits = cartesian_traits_base<
        PowN,
        cartesian_kind::power,
        read_kind::map,
        Base
    >;
    friend flux_sequence_traits::impl;
};

template <std::size_t PowN>
struct cartesian_power_map_fn
{
    template <typename Func, adaptable_sequence Seq>
        requires multipass_sequence<Seq> &&
        detail::repeated_invocable<Func&, element_t<Seq>, PowN>
    [[nodiscard]]
    constexpr auto operator()(Func func, Seq&& seq) const
    {
        if constexpr(PowN == 0) {
            return empty<std::invoke_result_t<Func>>;
        } else {
            return cartesian_power_map_adaptor<PowN, Func, std::decay_t<Seq>>(
                std::move(func), FLUX_FWD(seq));
        }
    }
};

} // namespace detail

FLUX_EXPORT
template <distance_t N>
    requires (N >= 0)
inline constexpr auto cartesian_power_map = detail::cartesian_power_map_fn<N>{};

} // namespace flux

#endif
