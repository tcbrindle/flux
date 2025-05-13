
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_CARTESIAN_POWER_MAP_HPP_INCLUDED
#define FLUX_ADAPTOR_CARTESIAN_POWER_MAP_HPP_INCLUDED

#include <flux/adaptor/cartesian_base.hpp>

namespace flux {

namespace detail {

template <sequence Base, std::size_t PowN, typename Func>
struct cartesian_power_map_adaptor
    : inline_sequence_base<cartesian_power_map_adaptor<Base, PowN, Func>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

public:
    constexpr explicit cartesian_power_map_adaptor(decays_to<Base> auto&& base, decays_to<Func> auto&& func)
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
    template <adaptable_sequence Seq, typename Func>
        requires multipass_sequence<Seq> &&
        detail::repeated_invocable<Func&, element_t<Seq>, PowN>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func) const
    {
        if constexpr(PowN == 0) {
            return empty<std::invoke_result_t<Func>>;
        } else {
            return cartesian_power_map_adaptor<std::decay_t<Seq>, PowN, Func>(
                FLUX_FWD(seq), std::move(func));
        }
    }
};

} // namespace detail

FLUX_EXPORT
template <int_t N>
    requires(N >= 0)
inline constexpr auto cartesian_power_map = detail::cartesian_power_map_fn<N> {};

} // namespace flux

#endif // FLUX_ADAPTOR_CARTESIAN_POWER_MAP_HPP_INCLUDED
