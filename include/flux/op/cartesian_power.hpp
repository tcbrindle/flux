
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

#include <tuple>

namespace flux {

namespace detail {

template <std::size_t RepeatCount, typename Base>
struct cartesian_power_traits_base;

template <std::size_t RepeatCount, sequence Base>
struct cartesian_power_adaptor
    : inline_sequence_base<cartesian_power_adaptor<RepeatCount, Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;

    friend struct cartesian_power_traits_base<RepeatCount, Base>;
    friend struct sequence_traits<cartesian_power_adaptor>;

public:
    constexpr explicit cartesian_power_adaptor(Base&& base)
        : base_(FLUX_FWD(base))
    {}
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


template <typename T, std::size_t RepeatCount>
constexpr auto repeat_tuple(T value) {
  return [value]<std::size_t... Is>(std::index_sequence<Is...>) {
    return std::tuple{(static_cast<void>(Is), value)...};
  }(std::make_index_sequence<RepeatCount>{});
}

template <typename T, std::size_t RepeatCount>
struct TupleRepeated {
  using type = decltype(repeat_tuple<T, RepeatCount>(std::declval<T>()));
};

template <typename T, std::size_t RepeatCount>
using tuple_repeated_t = TupleRepeated<T, RepeatCount>::type;


template <std::size_t PowN, typename Base>
struct cartesian_power_traits_base :
        cartesian_traits_base<cartesian_power_traits_base<PowN, Base>, Base>
{
private:

    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    template <typename Self>
    using cursor_type = std::array<cursor_t<const_like_t<Self, Base>>, PowN>;

    template<std::size_t I, typename Self>
    static constexpr auto&& get_base(Self& self) {
        return self.base_;
    }

    static consteval auto get_arity() {
        return PowN;
    }

    using this_type = cartesian_power_traits_base<PowN, Base>;

protected:
    using traits_base = cartesian_traits_base<this_type, Base>;
    friend traits_base;

public:

    template <typename Self>
    static constexpr auto first(Self& self) -> cursor_type<Self>
    {
      return []<std::size_t... Is>(auto&& arg, std::index_sequence<Is...>) {
          cursor_type<Self> cur = {(static_cast<void>(Is), flux::first(FLUX_FWD(arg)))...};
          return cur;
      }(self.base_, std::make_index_sequence<PowN>{});
    }

    template <typename Self>
    requires sized_sequence<const_like_t<Self, Base>>
    static constexpr auto size(Self& self) -> distance_t
    {
        return num::checked_pow(flux::size(self.base_), PowN);
    }
};

template <std::size_t PowN, typename Base>
struct cartesian_power_without_traits_base
        : cartesian_default_read_traits_base<cartesian_power_traits_base<PowN, Base>> {
};

} // end namespace detail

template <std::size_t PowN, typename Base>
struct sequence_traits<detail::cartesian_power_adaptor<PowN, Base>>
    : detail::cartesian_power_without_traits_base<PowN, Base>
{
    using value_type = detail::tuple_repeated_t<value_t<Base>, PowN>;
};

template<std::size_t PowN>
FLUX_EXPORT inline constexpr auto cartesian_power = detail::cartesian_power_fn<PowN>{};

} // end namespace flux

#endif

