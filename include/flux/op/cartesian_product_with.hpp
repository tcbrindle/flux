
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_PRODUCT_WITH_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_PRODUCT_WITH_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>

#include <tuple>

namespace flux {

namespace detail {

template <typename Func, lens... Bases>
struct cartesian_product_with_adaptor
    : lens_base<cartesian_product_with_adaptor<Func, Bases...>> {
private:
    FLUX_NO_UNIQUE_ADDRESS std::tuple<Bases...> bases_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

    friend struct sequence_iface<cartesian_product_with_adaptor>;

public:
    constexpr explicit cartesian_product_with_adaptor(Func func, Bases&&... bases)
        : bases_(std::move(bases)...),
          func_(std::move(func))
    {}
};

struct cartesian_product_with_fn
{
    template <typename Func, adaptable_sequence Seq0, adaptable_sequence... Seqs>
        requires (multipass_sequence<Seqs> && ...) &&
        std::regular_invocable<Func&, element_t<Seq0>, element_t<Seqs>...>
    constexpr auto operator()(Func func, Seq0&& seq0, Seqs&&... seqs) const
    {
        return cartesian_product_with_adaptor(std::move(func),
                                              flux::from(FLUX_FWD(seq0)),
                                              flux::from(FLUX_FWD(seqs))...);
    }
};

template <typename B0, typename...>
inline constexpr bool cartesian_product_is_bounded = bounded_sequence<B0>;

} // namespace detail

template <typename Func, typename... Bases>
struct sequence_iface<detail::cartesian_product_with_adaptor<Func, Bases...>> {

    using distance_type = std::common_type_t<distance_t<Bases>...>;

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
    static constexpr auto read_at(Self& self, cursor_type<Self> const& cur)
        -> std::invoke_result_t<const_like_t<Self, Func>&, element_t<const_like_t<Self, Bases>>...>
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) {
            return std::invoke(self.func_, flux::read_at(std::get<N>(self.bases_), std::get<N>(cur))...);
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_type<Self>& cur) -> cursor_type<Self>&
    {
        return inc_impl<sizeof...(Bases) - 1>(self, cur);
    }

    template <typename Self>
        requires detail::cartesian_product_is_bounded<const_like_t<Self, Bases>...>
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
    static constexpr auto inc(Self& self, cursor_type<Self>& cur, distance_type offset)
        -> cursor_type<Self>&
    {
        return inc_impl<0>(self, cur, offset);
    }

    template <typename Self>
        requires (sized_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto size(Self& self)
    {
        return std::apply([](auto&... base) {
            return (flux::size(base) * ...);
        }, self.bases_);
    }
};

inline constexpr auto cartesian_product_with = detail::cartesian_product_with_fn{};

} // namespace flux

#endif
