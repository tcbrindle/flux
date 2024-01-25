
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_PRODUCT_HPP_INCLUDED
#define FLUX_OP_PRODUCT_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/core/numeric.hpp>
#include <flux/op/from.hpp>

#include <tuple>

namespace flux {

namespace detail {

template <std::size_t RepeatCount, typename Base>
struct product_traits_base;

template <std::size_t RepeatCount, sequence Base>
struct product_adaptor
    : inline_sequence_base<product_adaptor<RepeatCount, Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;

    friend struct product_traits_base<RepeatCount, Base>;
    friend struct sequence_traits<product_adaptor>;

public:
    static constexpr std::size_t count_ = RepeatCount;
    constexpr explicit product_adaptor(Base&& base)
        : base_(FLUX_FWD(base))
    {}
};



template<std::size_t RepeatCount>
struct product_fn {

    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        return product_adaptor<RepeatCount, std::decay_t<Seq>>(
                    FLUX_FWD(seq));
    }
};

template <typename B0, typename...>
inline constexpr bool product_is_bounded = bounded_sequence<B0>;


template<typename T, std::size_t RepeatCount>
constexpr auto repeat_tuple(T value) {
  return [value]<std::size_t... Is>(std::index_sequence<Is...>) {
    return std::tuple{(static_cast<void>(Is), value)...};
  }(std::make_index_sequence<RepeatCount>{});
}

template<typename T, std::size_t RepeatCount>
struct TupleRepeated {

  using type = decltype(repeat_tuple<T, RepeatCount>(std::declval<T>()));

};

template<typename T, std::size_t RepeatCount>
using tuple_repeated_t = TupleRepeated<T, RepeatCount>::type;


template <std::size_t RepeatCount, typename Base>
struct product_traits_base {
private:
    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    template <typename Self>
    using cursor_type = tuple_repeated_t<cursor_t<const_like_t<Self, Base>>, RepeatCount>;


    template <std::size_t I, typename Self>
    static constexpr auto inc_impl(Self& self, cursor_type<Self>& cur) -> cursor_type<Self>&
    {
        flux::inc(self.base_, std::get<I>(cur));

        if constexpr (I > 0) {
            if (flux::is_last(self.base_, std::get<I>(cur))) {
                std::get<I>(cur) = flux::first(self.base_);
                inc_impl<I-1>(self, cur);
            }
        }

        return cur;
    }

    template <std::size_t I, typename Self>
    static constexpr auto dec_impl(Self& self, cursor_type<Self>& cur) -> cursor_type<Self>&
    {
        if (std::get<I>(cur) == flux::first(self.base_)) {
            std::get<I>(cur) = flux::last(self.base_);
            if constexpr (I > 0) {
                dec_impl<I-1>(self, cur);
            }
        }

        flux::dec(self.base_, std::get<I>(cur));

        return cur;
    }

    template <std::size_t I, typename Self>
    static constexpr auto ra_inc_impl(Self& self, cursor_type<Self>& cur, distance_t offset)
        -> cursor_type<Self>&
    {
        if (offset == 0)
            return cur;

        auto& base = self.base_;
        const auto this_index = flux::distance(base, flux::first(base), std::get<I>(cur));
        auto new_index = num::checked_add(this_index, offset);
        auto this_size = flux::size(base);

        // If the new index overflows the maximum or underflows zero, calculate the carryover and fix it.
        if (new_index < 0 || new_index >= this_size) {
            offset = num::checked_div(new_index, this_size);
            new_index = num::checked_mod(new_index, this_size);

            // Correct for negative index which may happen when underflowing.
            if (new_index < 0) {
                new_index = num::checked_add(new_index, this_size);
                offset = num::checked_sub(offset, flux::distance_t(1));
            }

            // Call the next level down if necessary.
            if constexpr (I > 0) {
                if (offset != 0) {
                    ra_inc_impl<I-1>(self, cur, offset);
                }
            }
        }

        flux::inc(base, std::get<I>(cur), num::checked_sub(new_index, this_index));

        return cur;
    }

    template <std::size_t I, typename Self>
    static constexpr auto distance_impl(Self& self,
                                        cursor_type<Self> const& from,
                                        cursor_type<Self> const& to) -> distance_t
    {
        if constexpr (I == 0) {
            return flux::distance(self.base_, std::get<0>(from), std::get<0>(to));
        } else {
            auto prev_dist = distance_impl<I-1>(self, from, to);
            auto our_sz = flux::size(self.base_);
            auto our_dist = flux::distance(self.base_, std::get<I>(from), std::get<I>(to));
            return prev_dist * our_sz + our_dist;
        }
    }

public:

    template <typename Self>
    static constexpr auto first(Self& self) -> cursor_type<Self>
    {
      return []<std::size_t... Is>(auto&& arg, std::index_sequence<Is...>) {
        return cursor_type<Self>((static_cast<void>(Is), flux::first(arg))...);
      }(self.base_, std::make_index_sequence<RepeatCount>{});
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, cursor_type<Self> const& cur) -> bool
    {
      return [&]<std::size_t... N>(std::index_sequence<N...>) {
        return (flux::is_last(self.base_, std::get<N>(cur)) || ...);
      }(std::make_index_sequence<RepeatCount>{});
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_type<Self>& cur) -> cursor_type<Self>&
    {
        return inc_impl<RepeatCount - 1>(self, cur);
    }

    template <typename Self>
        requires product_is_bounded<const_like_t<Self, Base>>
    static constexpr auto last(Self& self) -> cursor_type<Self>
    {
        auto cur = first(self);
        std::get<0>(cur) = flux::last(self.base_);
        return cur;
    }

    template <typename Self>
        requires (bidirectional_sequence<const_like_t<Self, Base>>) &&
                 (bounded_sequence<const_like_t<Self, Base>>)
    static constexpr auto dec(Self& self, cursor_type<Self>& cur) -> cursor_type<Self>&
    {
        return dec_impl<RepeatCount - 1>(self, cur);
    }

    template <typename Self>
        requires (random_access_sequence<const_like_t<Self, Base>>) &&
                 (sized_sequence<const_like_t<Self, Base>>)
    static constexpr auto inc(Self& self, cursor_type<Self>& cur, distance_t offset)
        -> cursor_type<Self>&
    {
        return ra_inc_impl<RepeatCount - 1>(self, cur, offset);
    }

    template <typename Self>
        requires (random_access_sequence<const_like_t<Self, Base>>) &&
                 (sized_sequence<const_like_t<Self, Base>>)
    static constexpr auto distance(Self& self,
                                   cursor_type<Self> const& from,
                                   cursor_type<Self> const& to) -> distance_t
    {
        return distance_impl<RepeatCount - 1>(self, from, to);
    }

    template <typename Self>
        requires (sized_sequence<const_like_t<Self, Base>>)
    static constexpr auto size(Self& self) -> distance_t
    {
      auto single_size = flux::size(self.base_);
      auto ret = single_size;
      for (std::size_t i{1}; i < self.count_; i++) {
        ret = ret * single_size;
      }
      return ret;
    }
};

} // end namespace detail

template <std::size_t RepeatCount, typename Base>
struct sequence_traits<detail::product_adaptor<RepeatCount, Base>>
    : detail::product_traits_base<RepeatCount, Base>
{
private:

    template <std::size_t I, typename Self, typename Fn>
    static constexpr auto read1_(Fn fn, Self& self, cursor_t<Self> const& cur)
    -> decltype(auto)
    {
      return fn(self.base_, std::get<I>(cur));
    }

    template <typename Fn, typename Self>
    static constexpr auto read_(Fn fn, Self& self, cursor_t<Self> const& cur)
    {
      return [&]<std::size_t... N>(std::index_sequence<N...>) {
        return std::tuple<decltype(read1_<N>(fn, self, cur))...>(read1_<N>(fn, self, cur)...);
      }(std::make_index_sequence<RepeatCount>{});
    }

    template <std::size_t I, typename Self, typename Function,
              typename... PartialElements>
    static constexpr void for_each_while_impl(Self& self,
                                              bool& keep_going,
                                              cursor_t<Self>& cur,
                                              Function&& func,
                                              PartialElements&&... partial_elements)
    {
        // We need to iterate right to left.
        if constexpr (I == RepeatCount - 1) {
            std::get<I>(cur) = flux::for_each_while(self.base_,
                [&](auto&& elem) {
                    keep_going = std::invoke(func,
                        element_t<Self>(FLUX_FWD(partial_elements)..., FLUX_FWD(elem)));
                    return keep_going;
                });
        } else {
            std::get<I>(cur) = flux::for_each_while(self.base_,
                [&](auto&& elem) {
                    for_each_while_impl<I+1>(
                        self, keep_going, cur,
                        func, FLUX_FWD(partial_elements)..., FLUX_FWD(elem));
                    return keep_going;
                });
        }
    }


public:
    using value_type = detail::tuple_repeated_t<value_t<Base>, RepeatCount>;

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::read_at, self, cur);
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::move_at, self, cur);
    }

    template <typename Self>
    static constexpr auto read_at_unchecked(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::read_at_unchecked, self, cur);
    }

    template <typename Self>
    static constexpr auto move_at_unchecked(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::move_at_unchecked, self, cur);
    }

    template <typename Self, typename Function>
    static constexpr auto for_each_while(Self& self, Function&& func)
        -> cursor_t<Self>
    {
        bool keep_going = true;
        cursor_t<Self> cur;
        for_each_while_impl<0>(self, keep_going, cur, FLUX_FWD(func));
        return cur;
    }
};

template<std::size_t RepeatCount>
FLUX_EXPORT inline constexpr auto products = detail::product_fn<RepeatCount>{};

} // end namespace flux

#endif

