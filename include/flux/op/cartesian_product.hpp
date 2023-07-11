
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_PRODUCT_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_PRODUCT_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/core/numeric.hpp>
#include <flux/op/from.hpp>

#include <tuple>

namespace flux {

namespace detail {

template <typename... Bases>
struct cartesian_product_traits_base;

template <sequence... Bases>
struct cartesian_product_adaptor
    : inline_sequence_base<cartesian_product_adaptor<Bases...>> {
private:
    FLUX_NO_UNIQUE_ADDRESS std::tuple<Bases...> bases_;

    friend struct cartesian_product_traits_base<Bases...>;
    friend struct sequence_traits<cartesian_product_adaptor>;

public:
    constexpr explicit cartesian_product_adaptor(decays_to<Bases> auto&&... bases)
        : bases_(FLUX_FWD(bases)...)
    {}
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

template <typename B0, typename...>
inline constexpr bool cartesian_product_is_bounded = bounded_sequence<B0>;

template <typename... Bases>
struct cartesian_product_traits_base {
private:
    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    template <typename Self>
    using cursor_type = std::tuple<cursor_t<const_like_t<Self, Bases>>...>;

    template <std::size_t I, typename Self>
    static constexpr auto inc_impl(Self& self, cursor_type<Self>& cur) -> cursor_type<Self>&
    {
        flux::inc(std::get<I>(self.bases_), std::get<I>(cur));

        if constexpr (I > 0) {
            if (flux::is_last(std::get<I>(self.bases_), std::get<I>(cur))) {
                std::get<I>(cur) = flux::first(std::get<I>(self.bases_));
                inc_impl<I-1>(self, cur);
            }
        }

        return cur;
    }

    template <std::size_t I, typename Self>
    static constexpr auto dec_impl(Self& self, cursor_type<Self>& cur) -> cursor_type<Self>&
    {
        if (std::get<I>(cur) == flux::first(std::get<I>(self.bases_))) {
            std::get<I>(cur) = flux::last(std::get<I>(self.bases_));
            if constexpr (I > 0) {
                dec_impl<I-1>(self, cur);
            }
        }

        flux::dec(std::get<I>(self.bases_), std::get<I>(cur));

        return cur;
    }

    template <std::size_t I, typename Self>
    static constexpr auto ra_inc_impl(Self& self, cursor_type<Self>& cur, distance_t offset)
        -> cursor_type<Self>&
    {
        if (offset == 0)
            return cur;

        auto& base = std::get<I>(self.bases_);
        auto& this_index = std::get<I>(cur);
        auto this_size = flux::size(base);

        this_index += offset;

        if (this_index >= 0 && this_index < this_size)
            return cur;

        // If the new index overflows the maximum or underflows zero, calculate the carryover and fix it.
        else {
            offset = this_index / this_size;
            this_index %= this_size;

            // Correct for negative index which may happen when underflowing.
            if (this_index < 0) {
                this_index += this_size;
                --offset;
            }

            // Call the next level down if necessary.
            if constexpr (I > 0) {
                if (offset != 0) {
                    ra_inc_impl<I-1>(self, cur, offset);
                }
            }
        }

        return cur;
    }

    template <std::size_t I, typename Self>
    static constexpr auto distance_impl(Self& self,
                                        cursor_type<Self> const& from,
                                        cursor_type<Self> const& to) -> distance_t
    {
        if constexpr (I == 0) {
            return flux::distance(std::get<0>(self.bases_), std::get<0>(from), std::get<0>(to));
        } else {
            auto prev_dist = distance_impl<I-1>(self, from, to);
            auto our_sz = flux::size(std::get<I>(self.bases_));
            auto our_dist = flux::distance(std::get<I>(self.bases_), std::get<I>(from), std::get<I>(to));
            return prev_dist * our_sz + our_dist;
        }
    }

public:
    template <typename Self>
    static constexpr auto first(Self& self) -> cursor_type<Self>
    {
        return std::apply([](auto&&... args) {
          return cursor_type<Self>(flux::first(FLUX_FWD(args))...);
        }, self.bases_);
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, cursor_type<Self> const& cur) -> bool
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) {
          return (flux::is_last(std::get<N>(self.bases_), std::get<N>(cur)) || ...);
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_type<Self>& cur) -> cursor_type<Self>&
    {
        return inc_impl<sizeof...(Bases) - 1>(self, cur);
    }

    template <typename Self>
        requires cartesian_product_is_bounded<const_like_t<Self, Bases>...>
    static constexpr auto last(Self& self) -> cursor_type<Self>
    {
        auto cur = first(self);
        std::get<0>(cur) = flux::last(std::get<0>(self.bases_));
        return cur;
    }

    template <typename Self>
        requires (bidirectional_sequence<const_like_t<Self, Bases>> && ...) &&
                 (bounded_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto dec(Self& self, cursor_type<Self>& cur) -> cursor_type<Self>&
    {
        return dec_impl<sizeof...(Bases) - 1>(self, cur);
    }

    template <typename Self>
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...) &&
                 (sized_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto inc(Self& self, cursor_type<Self>& cur, distance_t offset)
        -> cursor_type<Self>&
    {
        return ra_inc_impl<sizeof...(Bases) - 1>(self, cur, offset);
    }

    template <typename Self>
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...) &&
                 (sized_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto distance(Self& self,
                                   cursor_type<Self> const& from,
                                   cursor_type<Self> const& to) -> distance_t
    {
        return distance_impl<sizeof...(Bases) - 1>(self, from, to);
    }

    template <typename Self>
        requires (sized_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto size(Self& self) -> distance_t
    {
        return std::apply([](auto&... base) {
          return (flux::size(base) * ...);
        }, self.bases_);
    }
};

template <typename Self, typename I>
struct cartesian_product_partial_cursor;

template <typename Self>
struct cartesian_product_partial_cursor<Self,
    std::integral_constant<std::size_t, std::tuple_size_v<cursor_t<Self>> - 1>>
{
    using type = std::tuple<std::tuple_element_t<std::tuple_size_v<cursor_t<Self>> - 1, cursor_t<Self>>>;
};

template <typename Self, typename I>
struct cartesian_product_partial_cursor
{
    using type = decltype(std::tuple_cat(
        std::declval<std::tuple<std::tuple_element_t<I::value, cursor_t<Self>>>>(),
        std::declval<typename cartesian_product_partial_cursor<Self,
            std::integral_constant<std::size_t, I::value + 1>>::type>()));
};

template <typename Self, std::size_t I>
using cartesian_product_partial_cursor_t =
    typename cartesian_product_partial_cursor<Self, std::integral_constant<std::size_t, I>>::type;


} // end namespace detail

template <typename... Bases>
struct sequence_traits<detail::cartesian_product_adaptor<Bases...>>
    : detail::cartesian_product_traits_base<Bases...>
{
private:
    template <std::size_t I, typename Self, typename Fn>
    static constexpr auto read1_(Fn fn, Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
    {
        return fn(std::get<I>(self.bases_), std::get<I>(cur));
    }

    template <typename Fn, typename Self>
    static constexpr auto read_(Fn fn, Self& self, cursor_t<Self> const& cur)
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) {
            return std::tuple<decltype(read1_<N>(fn, self, cur))...>(read1_<N>(fn, self, cur)...);
        }(std::index_sequence_for<Bases...>{});
    }

    template <std::size_t I, typename Self, typename Function,
              typename... PartialCursor>
    static constexpr auto for_each_while_impl(Self& self,
                                              Function&& func,
                                              PartialCursor&&... partial_cursor)
        -> std::tuple<bool, detail::cartesian_product_partial_cursor_t<Self, I>>
    {
        // We need to iterate right to left.
        if constexpr (I == sizeof...(Bases) - 1) {
            bool keep_going = true;
            auto this_current = flux::for_each_while(std::get<I>(self.bases_),
                [&](auto&& elem) {
                    keep_going = std::invoke(func,
                        cursor_t<Self>(FLUX_FWD(partial_cursor)..., FLUX_FWD(elem)));
                    return keep_going;
                });
            return std::tuple(keep_going, std::tuple(std::move(this_current)));
        } else {
            bool keep_going = true;
            detail::cartesian_product_partial_cursor_t<Self, I+1> nested_current;
            auto this_current = flux::for_each_while(std::get<I>(self.bases_),
                [&](auto&& elem) {
                    std::tie(keep_going, nested_current) = for_each_while_impl<I+1>(
                        self, func, FLUX_FWD(partial_cursor)..., FLUX_FWD(elem));
                    return keep_going;
                });
            return std::tuple(keep_going,
                std::tuple_cat(std::tuple(std::move(this_current)), std::move(nested_current)));
        }
    }

public:
    using value_type = std::tuple<value_t<Bases>...>;

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
        return std::get<1>(for_each_while_impl<0>(self, FLUX_FWD(func)));
    }
};

FLUX_EXPORT inline constexpr auto cartesian_product = detail::cartesian_product_fn{};

} // end namespace flux

#endif

