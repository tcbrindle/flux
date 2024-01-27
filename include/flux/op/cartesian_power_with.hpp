
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_POWER_WITH_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_POWER_WITH_HPP_INCLUDED

#include <flux/op/cartesian_power.hpp>

namespace flux {

namespace detail {

template <std::size_t PowN, typename Base>
struct cartesian_power_with_traits_base
        : cartesian_power_traits_base<PowN, Base> {
};

template <std::size_t PowN, typename Func, sequence Base>
struct cartesian_power_with_adaptor
    : inline_sequence_base<cartesian_power_with_adaptor<PowN, Func, Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

    friend struct cartesian_power_traits_base<PowN, Base>;
    friend struct sequence_traits<cartesian_power_with_adaptor>;

public:
    constexpr explicit cartesian_power_with_adaptor(decays_to<Func> auto&& func, decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base)),
          func_(FLUX_FWD(func))
    {}

    struct flux_sequence_traits : detail::cartesian_power_with_traits_base<PowN, Base>
    {
        template <typename Self>
        static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
            -> decltype(auto)
        {
            return [&]<std::size_t... N>(std::index_sequence<N...>) -> decltype(auto) {
                return std::invoke(self.func_, flux::read_at(self.base_, std::get<N>(cur))...);
            }(std::make_index_sequence<PowN>{});
        }
    };
};


template <typename F, typename Arg, std::size_t RepeatCount, typename... Args>
struct invocable_repeated_trait :
        std::integral_constant<bool, invocable_repeated_trait<F, Arg, RepeatCount - 1, Arg, Args...>::value> {
};

template <typename F, typename Arg, typename... Args>
struct invocable_repeated_trait<F, Arg, 0, Args...> :
        std::integral_constant<bool, std::regular_invocable<F, Args...>> {
};

template <typename F, typename Arg, std::size_t RepeatCount>
concept regular_invocable_repeated = invocable_repeated_trait<F, Arg, RepeatCount>::value;

template <std::size_t PowN>
struct cartesian_power_with_fn
{
    template <typename Func, adaptable_sequence Seq>
        requires multipass_sequence<Seq> &&
        regular_invocable_repeated<Func&, element_t<Seq>, PowN>
    [[nodiscard]]
    constexpr auto operator()(Func func, Seq&& seq) const
    {
        return cartesian_power_with_adaptor<PowN, Func, std::decay_t<Seq>>(
                    std::move(func), FLUX_FWD(seq));
    }
};

} // namespace detail

template <std::size_t PowN>
FLUX_EXPORT inline constexpr auto cartesian_power_with = detail::cartesian_power_with_fn<PowN>{};

} // namespace flux

#endif
