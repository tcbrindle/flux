
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_PRODUCT_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_PRODUCT_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>

#include <tuple>

namespace flux {

namespace detail {

template <typename... Bases>
struct cartesian_product_traits_base;

template <sequence... Bases>
struct cartesian_product_adaptor
    : lens_base<cartesian_product_adaptor<Bases...>> {
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
        auto& base = std::get<I>(self.bases_);

        auto this_sz = flux::size(base);
        auto this_offset = offset  % this_sz;
        auto next_offset = offset/this_sz;

        // Adjust this cursor by the corrected offset
        flux::inc(base, std::get<I>(cur), this_offset);

        // Call the next level down if necessary
        if constexpr (I > 0) {
            if (next_offset != 0) {
                ra_inc_impl<I-1>(self, cur, next_offset);
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


} // end namespace detail

template <typename... Bases>
struct sequence_traits<detail::cartesian_product_adaptor<Bases...>>
    : detail::cartesian_product_traits_base<Bases...>
{
private:
    template <std::size_t I, typename Self>
    static constexpr auto read_at_impl(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
    {
        return flux::read_at(std::get<I>(self.bases_), std::get<I>(cur));
    }

    template <std::size_t I, typename Self>
    static constexpr auto move_at_impl(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
    {
        return flux::move_at(std::get<I>(self.bases_), std::get<I>(cur));
    }

public:
    using value_type = std::tuple<value_t<Bases>...>;

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) {
            return std::tuple<decltype(read_at_impl<N>(self, cur))...>(read_at_impl<N>(self, cur)...);
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) {
          return std::tuple<decltype(move_at_impl<N>(self, cur))...>(move_at_impl<N>(self, cur)...);
        }(std::index_sequence_for<Bases...>{});
    }
};

inline constexpr auto cartesian_product = detail::cartesian_product_fn{};

} // end namespace flux

#endif

