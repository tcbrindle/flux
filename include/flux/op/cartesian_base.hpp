
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CARTESIAN_BASE_HPP_INCLUDED
#define FLUX_CARTESIAN_BASE_HPP_INCLUDED

namespace flux::detail {

enum class cartesian_kind { product, power };
enum class read_kind { tuple, map };

template <typename B0, typename...>
inline constexpr bool cartesian_is_bounded = bounded_sequence<B0>;

template <typename T, std::size_t RepeatCount>
struct tuple_repeated {
    template <std::size_t I>
    using repeater = T;

    template <std::size_t... Is>
    static auto make_tuple(std::index_sequence<Is...>) -> std::tuple<repeater<Is>...>;    

    using type = decltype(make_tuple(std::make_index_sequence<RepeatCount>{}));
};

template <typename T, std::size_t RepeatCount>
using tuple_repeated_t = tuple_repeated<T, RepeatCount>::type;

template<std::size_t Arity, cartesian_kind CartesianKind, read_kind ReadKind, typename... Bases>
struct cartesian_traits_types {
};

template<std::size_t Arity, typename Base>
struct cartesian_traits_types<Arity, cartesian_kind::power, read_kind::tuple, Base> {
    using value_type = tuple_repeated_t<value_t<Base>, Arity>;
};

template<std::size_t Arity, typename... Bases>
struct cartesian_traits_types<Arity, cartesian_kind::product, read_kind::tuple, Bases...> {
    using value_type = std::tuple<value_t<Bases>...>;
};

template <std::size_t Arity, cartesian_kind CartesianKind, read_kind ReadKind, typename... Bases>
struct cartesian_traits_base_impl : default_sequence_traits {
private:

    template<std::size_t I, typename Self>
    static constexpr auto& get_base(Self& self)
        requires (CartesianKind == cartesian_kind::power)
    {
        return self.base_;
    }

    template<std::size_t I, typename Self>
    static constexpr auto& get_base(Self& self)
        requires (CartesianKind == cartesian_kind::product)
    {
        return std::get<I>(self.bases_);
    }


    template <std::size_t I, typename Self>
    static constexpr auto inc_impl(Self& self, cursor_t<Self>& cur) -> cursor_t<Self>&
    {
        flux::inc(get_base<I>(self), std::get<I>(cur));

        if constexpr (I > 0) {
            if (flux::is_last(get_base<I>(self), std::get<I>(cur))) {
                std::get<I>(cur) = flux::first(get_base<I>(self));
                inc_impl<I-1>(self, cur);
            }
        }

        return cur;
    }


    template <std::size_t I, typename Self>
    static constexpr auto dec_impl(Self& self, cursor_t<Self>& cur) -> cursor_t<Self>&
    {
        if (std::get<I>(cur) == flux::first(get_base<I>(self))) {
            std::get<I>(cur) = flux::last(get_base<I>(self));
            if constexpr (I > 0) {
                dec_impl<I-1>(self, cur);
            }
        }

        flux::dec(get_base<I>(self), std::get<I>(cur));

        return cur;
    }

    template <std::size_t I, typename Self>
    static constexpr auto ra_inc_impl(Self& self, cursor_t<Self>& cur, distance_t offset)
    -> cursor_t<Self>&
    {
        if (offset == 0) {
            return cur;
        }

        auto& base = get_base<I>(self);
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
                                        cursor_t<Self> const& from,
                                        cursor_t<Self> const& to) -> distance_t
    {
        if constexpr (I == 0) {
            return flux::distance(get_base<0>(self), std::get<0>(from), std::get<0>(to));
        } else {
            auto prev_dist = distance_impl<I-1>(self, from, to);
            auto our_sz = flux::size(get_base<I>(self));
            auto our_dist = flux::distance(get_base<I>(self), std::get<I>(from), std::get<I>(to));
            return prev_dist * our_sz + our_dist;
        }
    }

    template <std::size_t I, typename Self, typename Fn>
    static constexpr auto read1_(Fn fn, Self& self, cursor_t<Self> const& cur)
    -> decltype(auto)
    {
        return fn(get_base<I>(self), std::get<I>(cur));
    }

    template <typename Fn, typename Self>
    static constexpr auto read_(Fn& fn, Self& self, cursor_t<Self> const& cur)
    -> decltype(auto)
        requires (ReadKind == read_kind::tuple)
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) {
            return std::tuple<decltype(read1_<N>(fn, self, cur))...>(read1_<N>(fn, self, cur)...);
        }(std::make_index_sequence<Arity>{});
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
        if constexpr (I == Arity - 1) {
            std::get<I>(cur) = flux::for_each_while(get_base<I>(self),
                [&](auto&& elem) {
                    keep_going = std::invoke(func,
                                             element_t<Self>(FLUX_FWD(partial_elements)..., FLUX_FWD(elem)));
                    return keep_going;
                });
        } else {
            std::get<I>(cur) = flux::for_each_while(get_base<I>(self),
                [&](auto&& elem) {
                    for_each_while_impl<I+1>(
                            self, keep_going, cur,
                            func, FLUX_FWD(partial_elements)..., FLUX_FWD(elem));
                    return keep_going;
                });
        }
    }

protected:
    using types = cartesian_traits_types<Arity, CartesianKind, ReadKind, Bases...>;

public:

    template <typename Self>
    static constexpr auto first(Self& self)
        requires (CartesianKind == cartesian_kind::product)
    {
        return std::apply([](auto&&... args) {
            return std::tuple(flux::first(FLUX_FWD(args))...);
        }, self.bases_);
    }

    template <typename Self>
    static constexpr auto first(Self& self)
        requires (CartesianKind == cartesian_kind::power)
    {
        auto base_cur = flux::first(self.base_);
        return [&base_cur]<std::size_t... Is>(std::index_sequence<Is...>) {
            static_assert(sizeof...(Bases) == 1);
            std::array<cursor_t<Bases>..., Arity> cur = {(static_cast<void>(Is), base_cur)...};
            return cur;
        }(std::make_index_sequence<Arity>{});
    }

    template <typename Self>
    static constexpr auto size(Self& self) -> distance_t
        requires (CartesianKind == cartesian_kind::product
                  && (sized_sequence<Bases> && ...))
    {
        return std::apply([](auto& base0, auto&... bases) {
            distance_t sz = flux::size(base0);
            ((sz = num::checked_mul(sz, flux::size(bases))), ...);
            return sz;
        }, self.bases_);
    }

    template <typename Self>
    static constexpr auto size(Self& self) -> distance_t
        requires (CartesianKind == cartesian_kind::power
                  && (sized_sequence<Bases> && ...))
    {
        return num::checked_pow(flux::size(self.base_), Arity);
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, cursor_t<Self> const& cur) -> bool
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) {
            return (flux::is_last(get_base<N>(self), std::get<N>(cur)) || ...);
        }(std::make_index_sequence<Arity>{});
    }

    template <typename Self>
    static constexpr auto dec(Self& self, cursor_t<Self>& cur) -> cursor_t<Self>&
        requires ((bidirectional_sequence<Bases> && ...) &&
                  (bounded_sequence<Bases> && ...))
    {
        return dec_impl<Arity - 1>(self, cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur, distance_t offset) -> cursor_t<Self>&
        requires ((random_access_sequence<Bases> && ...) &&
                  (sized_sequence<Bases> && ...))
    {
        return ra_inc_impl<Arity - 1>(self, cur, offset);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur) -> cursor_t<Self>&
    {
        return inc_impl<Arity - 1>(self, cur);
    }

    template <typename Self>
    static constexpr auto last(Self& self) -> cursor_t<Self>
        requires cartesian_is_bounded<Bases...>
    {
        if constexpr (CartesianKind == cartesian_kind::product) {
            auto cur = first(self);
            bool any_is_empty = std::apply([](auto& /*ignored*/, auto&... bases) {
                    return (flux::is_empty(bases) || ...);
                }, self.bases_);
            if (!any_is_empty) {
                std::get<0>(cur) = flux::last(get_base<0>(self));
            }
            return cur;
        } else {
            auto cur = first(self);
            std::get<0>(cur) = flux::last(get_base<0>(self));
            return cur;
        }
    }

    template <typename Self>
    static constexpr auto distance(Self& self,
                                        cursor_t<Self> const& from,
                                        cursor_t<Self> const& to) -> distance_t
        requires ((random_access_sequence<Bases> && ...) &&
                  (sized_sequence<Bases> && ...))
    {
        return distance_impl<Arity - 1>(self, from, to);
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
        requires (ReadKind == read_kind::tuple)
    {
        return read_(flux::read_at, self, cur);
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
        requires (ReadKind == read_kind::map)
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) -> decltype(auto) {
            return std::invoke(self.func_, flux::read_at(get_base<N>(self), std::get<N>(cur))...);
        }(std::make_index_sequence<Arity>{});
    }

    template <typename Self>
    static constexpr auto read_at_unchecked(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
        requires (ReadKind == read_kind::map)
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) -> decltype(auto) {
            return std::invoke(self.func_, flux::read_at_unchecked(get_base<N>(self), std::get<N>(cur))...);
        }(std::make_index_sequence<Arity>{});
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
        requires (ReadKind == read_kind::map)
    {
        if constexpr (std::is_lvalue_reference_v<decltype(read_at(self, cur))>) {
            return std::move(read_at(self, cur));
        } else {
            return read_at(self, cur);
        }
    }

    template <typename Self>
    static constexpr auto move_at_unchecked(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
        requires (ReadKind == read_kind::map)
    {
        if constexpr (std::is_lvalue_reference_v<decltype(read_at_unchecked(self, cur))>) {
            return std::move(read_at_unchecked(self, cur));
        } else {
            return read_at_unchecked(self, cur);
        }
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
        requires (ReadKind == read_kind::tuple)
    {
        return read_(flux::move_at, self, cur);
    }

    template <typename Self>
    static constexpr auto read_at_unchecked(Self& self, cursor_t<Self> const& cur)
        requires (ReadKind == read_kind::tuple)
    {
        return read_(flux::read_at_unchecked, self, cur);
    }

    template <typename Self>
    static constexpr auto move_at_unchecked(Self& self, cursor_t<Self> const& cur)
        requires (ReadKind == read_kind::tuple)
    {
        return read_(flux::move_at_unchecked, self, cur);
    }

    template <typename Self, typename Function>
    static constexpr auto for_each_while(Self& self, Function&& func)
    -> cursor_t<Self>
        requires (ReadKind == read_kind::tuple)
    {
        bool keep_going = true;
        cursor_t<Self> cur;
        for_each_while_impl<0>(self, keep_going, cur, FLUX_FWD(func));
        return cur;
    }

};

template <std::size_t Arity, cartesian_kind CartesianKind, read_kind ReadKind, typename... Bases>
struct cartesian_traits_base : cartesian_traits_base_impl<Arity, CartesianKind, ReadKind, Bases...> {
    using impl = cartesian_traits_base_impl<Arity, CartesianKind, ReadKind, Bases...>;
};

template <std::size_t Arity, cartesian_kind CartesianKind, typename... Bases>
struct cartesian_traits_base<Arity, CartesianKind, read_kind::tuple, Bases...> : cartesian_traits_base_impl<Arity, CartesianKind, read_kind::tuple, Bases...> {
    using impl = cartesian_traits_base_impl<Arity, CartesianKind, read_kind::tuple, Bases...>;

    using value_type = typename impl::types::value_type;
};

}

#endif //FLUX_CARTESIAN_BASE_HPP_INCLUDED
