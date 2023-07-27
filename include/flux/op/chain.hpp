
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CHAIN_HPP_INCLUDED
#define FLUX_OP_CHAIN_HPP_INCLUDED

#include <flux/core.hpp>

#include <tuple>
#include <variant>

namespace flux {

namespace detail {

template <sequence... Bases>
struct chain_adaptor : inline_sequence_base<chain_adaptor<Bases...>> {
private:
    std::tuple<Bases...> bases_;

    friend struct sequence_traits<chain_adaptor>;

public:
    explicit constexpr chain_adaptor(decays_to<Bases> auto&&... bases)
        : bases_(FLUX_FWD(bases)...)
    {}
};

template <typename... Ts>
concept all_have_common_ref =
    requires { typename std::common_reference_t<Ts...>; } &&
    (std::convertible_to<Ts, std::common_reference_t<Ts...>> && ...);

template <typename... Seqs>
concept chainable =
    all_have_common_ref<element_t<Seqs>...> &&
    all_have_common_ref<rvalue_element_t<Seqs>...> &&
    requires { typename std::common_type_t<value_t<Seqs>...>; };

struct chain_fn {
    template <adaptable_sequence... Seqs>
        requires (sizeof...(Seqs) >= 1) &&
                 chainable<Seqs...>
    [[nodiscard]]
    constexpr auto operator()(Seqs&&... seqs) const
    {
        if constexpr (sizeof...(Seqs) == 1) {
            return std::forward<Seqs...>(seqs...);
        } else {
            return chain_adaptor<std::decay_t<Seqs>...>(FLUX_FWD(seqs)...);
        }
    }
};

} // namespace detail

template <typename... Bases>
struct sequence_traits<detail::chain_adaptor<Bases...>> {

    using value_type = std::common_type_t<value_t<Bases>...>;

    static constexpr bool disable_multipass = !(multipass_sequence<Bases> && ...);
    static constexpr bool is_infinite = (infinite_sequence<Bases> || ...);

private:
    static constexpr std::size_t End = sizeof...(Bases) - 1;

    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    using cursor_type = std::variant<cursor_t<Bases>...>;

    template <std::size_t N, typename Self>
    static constexpr auto first_impl(Self& self) -> cursor_type
    {
        auto& base = std::get<N>(self.bases_);
        auto cur = flux::first(base);

        if constexpr (N < End) {
            if (!flux::is_last(base, cur)) {
                return cursor_type(std::in_place_index<N>, std::move(cur));
            } else {
                return first_impl<N+1>(self);
            }
        } else {
            return cursor_type(std::in_place_index<N>, std::move(cur));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto inc_impl(Self& self, cursor_type& cur) -> void
    {
        if constexpr (N < End) {
            if (cur.index() == N) {
                auto& base = std::get<N>(self.bases_);
                auto& base_cur = std::get<N>(cur);
                flux::inc(base, base_cur);
                if (flux::is_last(base, base_cur)) {
                    cur = first_impl<N + 1>(self);
                }
            } else {
                inc_impl<N+1>(self, cur);
            }
        } else {
            flux::inc(std::get<N>(self.bases_), std::get<N>(cur));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto dec_impl(Self& self, cursor_type& cur)
    {
        if constexpr (N > 0) {
            if (cur.index() == N) {
                auto& base = std::get<N>(self.bases_);
                auto& base_cur = std::get<N>(cur);

                if (base_cur == flux::first(base)) {
                    cur = cursor_type(std::in_place_index<N-1>,
                                      flux::last(std::get<N-1>(self.bases_)));
                    dec_impl<N-1>(self, cur);
                } else {
                    flux::dec(base, base_cur);
                }
            } else {
                dec_impl<N-1>(self, cur);
            }
        } else {
            flux::dec(std::get<0>(self.bases_), std::get<0>(cur));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto read_impl(Self& self, cursor_type const& cur)
        -> std::common_reference_t<element_t<const_like_t<Self, Bases>>...>
    {
        if constexpr (N < End) {
            if (cur.index() == N) {
                return flux::read_at(std::get<N>(self.bases_), std::get<N>(cur));
            } else {
                return read_impl<N+1>(self, cur);
            }
        } else {
            return flux::read_at(std::get<N>(self.bases_), std::get<N>(cur));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto move_impl(Self& self, cursor_type const& cur)
        -> std::common_reference_t<rvalue_element_t<const_like_t<Self, Bases>>...>
    {
        if constexpr (N < End) {
            if (cur.index() == N) {
                return flux::move_at(std::get<N>(self.bases_), std::get<N>(cur));
            } else {
                return move_impl<N+1>(self, cur);
            }
        } else {
            return flux::move_at(std::get<N>(self.bases_), std::get<N>(cur));
        }
    }


    template <std::size_t N, typename Self>
    static constexpr auto for_each_while_impl(Self& self, auto& pred)
        -> cursor_type
    {
        if constexpr (N < End) {
            auto& base = std::get<N>(self.bases_);
            auto base_cur = flux::for_each_while(base, pred);
            if (!flux::is_last(base, base_cur)) {
                return cursor_type(std::in_place_index<N>, std::move(base_cur));
            } else {
                return for_each_while_impl<N+1>(self, pred);
            }
        } else {
            return cursor_type(std::in_place_index<N>,
                               flux::for_each_while(std::get<N>(self.bases_), pred));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto distance_impl(Self& self,
                                        cursor_type const& from,
                                        cursor_type const& to)
    {
        if constexpr (N < End) {
            if (N < from.index()) {
                return distance_impl<N+1>(self, from, to);
            }

            FLUX_DEBUG_ASSERT(N == from.index());
            if (N == to.index()) {
                return flux::distance(std::get<N>(self.bases_),
                                      std::get<N>(from), std::get<N>(to));
            } else {
                auto dist_to_end = flux::distance(std::get<N>(self.bases_),
                                                  std::get<N>(from),
                                                  flux::last(std::get<N>(self.bases_)));
                auto remaining = distance_impl<N+1>(self, first_impl<N+1>(self), to);
                return dist_to_end + remaining;
            }
        } else {
            FLUX_DEBUG_ASSERT(N == from.index() && N == to.index());
            return flux::distance(std::get<N>(self.bases_), std::get<N>(from), std::get<N>(to));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto inc_ra_impl(Self& self, cursor_type& cur,
                                      distance_t offset)
        -> cursor_type&
    {
        if constexpr (N < End) {
            if (N < cur.index()) {
                return inc_ra_impl<N+1>(self, cur, offset);
            }

            FLUX_DEBUG_ASSERT(cur.index() == N);
            auto& base = std::get<N>(self.bases_);
            auto& base_cur = std::get<N>(cur);
            auto dist = flux::distance(base, base_cur, flux::last(base));
            if (offset < dist) {
                flux::inc(base, base_cur, offset);
                return cur;
            } else {
                cur = first_impl<N+1>(self);
                offset -= dist;
                return inc_ra_impl<N+1>(self, cur, offset);
            }
        } else {
            FLUX_DEBUG_ASSERT(cur.index() == N);
            flux::inc(std::get<N>(self.bases_), std::get<N>(cur), offset);
            return cur;
        }
    }

public:
    template <typename Self>
    static constexpr auto first(Self& self) -> cursor_type
    {
        return first_impl<0>(self);
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
    {
        return cur.index() == End &&
               flux::is_last(std::get<End>(self.bases_), std::get<End>(cur));
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_type const& cur)
        -> std::common_reference_t<element_t<const_like_t<Self, Bases>>...>
    {
        return read_impl<0>(self, cur);
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_type const& cur)
        -> std::common_reference_t<rvalue_element_t<const_like_t<Self, Bases>>...>
    {
        return move_impl<0>(self, cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_type& cur) -> void
    {
        inc_impl<0>(self, cur);
    }

    template <typename Self>
    static constexpr auto dec(Self& self, cursor_type& cur) -> void
        requires (bidirectional_sequence<const_like_t<Self, Bases>> && ...) &&
                 (bounded_sequence<const_like_t<Self,Bases>> &&...)
    {
        dec_impl<End>(self, cur);
    }

    template <typename Self>
    static constexpr auto last(Self& self) -> cursor_type
        requires bounded_sequence<decltype(std::get<End>(self.bases_))>
    {
        return cursor_type(std::in_place_index<End>, flux::last(std::get<End>(self.bases_)));
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        requires (sized_sequence<const_like_t<Self, Bases>> && ...)
    {
        return std::apply([](auto&... bases) { return (flux::size(bases) + ...); },
                          self.bases_);
    }

    template <typename Self>
    static constexpr auto for_each_while(Self& self, auto&& pred)
    {
        return for_each_while_impl<0>(self, pred);
    }

    template <typename Self>
    static constexpr auto distance(Self& self, cursor_type const& from,
                                   cursor_type const& to)
        -> distance_t
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...) &&
                 (bounded_sequence<const_like_t<Self, Bases>> && ...)
    {
        if (from.index() <= to.index()) {
            return distance_impl<0>(self, from, to);
        } else {
            return -distance_impl<0>(self, to, from);
        }
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_type& cur, distance_t offset)
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...) &&
                 (bounded_sequence<const_like_t<Self, Bases>> && ...)
    {
        inc_ra_impl<0>(self, cur, offset);
    }

};

FLUX_EXPORT inline constexpr auto chain = detail::chain_fn{};

} // namespace flux

#endif
