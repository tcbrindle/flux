
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ZIP_HPP_INCLUDED
#define FLUX_OP_ZIP_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/source/empty.hpp>

namespace flux {

namespace detail {

template <typename... Ts>
struct pair_or_tuple {
    using type = std::tuple<Ts...>;
};

template <typename T, typename U>
struct pair_or_tuple<T, U> {
    using type = std::pair<T, U>;
};

template <typename... Ts>
using pair_or_tuple_t = typename pair_or_tuple<Ts...>::type;

template <sequence... Bases>
struct zip_adaptor : inline_sequence_base<zip_adaptor<Bases...>> {
private:
    pair_or_tuple_t<Bases...> bases_;

    friend struct sequence_traits<zip_adaptor>;

public:
    constexpr explicit zip_adaptor(decays_to<Bases> auto&&... bases)
        : bases_(FLUX_FWD(bases)...)
    {}
};

struct zip_fn {
    template <adaptable_sequence... Seqs>
    [[nodiscard]]
    constexpr auto operator()(Seqs&&... seqs) const
    {
        if constexpr (sizeof...(Seqs) == 0) {
            return empty<std::tuple<>>;
        } else {
            return zip_adaptor<std::decay_t<Seqs>...>(FLUX_FWD(seqs)...);
        }
    }
};

} // namespace detail

template <typename... Bases>
struct sequence_traits<detail::zip_adaptor<Bases...>>
{
private:
    template <typename... Ts>
    using tuple_t = detail::pair_or_tuple_t<Ts...>;

    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    template <std::size_t I>
    static constexpr decltype(auto) read_at_(auto& self, auto const& cur)
    {
        return flux::read_at(std::get<I>(self.bases_), std::get<I>(cur));
    }

    template <std::size_t I>
    static constexpr decltype(auto) move_at_(auto& self, auto const& cur)
    {
        return flux::move_at(std::get<I>(self.bases_), std::get<I>(cur));
    }

public:
    using value_type = tuple_t<value_t<Bases>...>;

    static constexpr bool is_infinite = (infinite_sequence<Bases> && ...);

    static constexpr auto first(auto& self)
    {
        return std::apply([](auto&&... args) {
            return tuple_t<decltype(flux::first(FLUX_FWD(args)))...>(flux::first(FLUX_FWD(args))...);
        }, self.bases_);
    }

    template <typename Self>
    static constexpr bool is_last(Self& self, cursor_t<Self> const& cur)
    {
        return [&self, &cur]<std::size_t... I>(std::index_sequence<I...>) {
            return (flux::is_last(std::get<I>(self.bases_), std::get<I>(cur)) || ...);
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return tuple_t<decltype(read_at_<I>(self, cur))...> {
                read_at_<I>(self, cur)...
            };
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto& inc(Self& self, cursor_t<Self>& cur)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::inc(std::get<I>(self.bases_), std::get<I>(cur)), ...);
        }(std::index_sequence_for<Bases...>{});

        return cur;
    }

    template <typename Self>
    static constexpr auto& dec(Self& self, cursor_t<Self>& cur)
        requires (bidirectional_sequence<const_like_t<Self, Bases>> && ...)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::dec(std::get<I>(self.bases_), std::get<I>(cur)), ...);
        }(std::index_sequence_for<Bases...>{});

        return cur;
    }

    template <typename Self>
    static constexpr auto& inc(Self& self, cursor_t<Self>& cur, distance_t offset)
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::inc(std::get<I>(self.bases_), std::get<I>(cur), offset), ...);
        }(std::index_sequence_for<Bases...>{});

        return cur;
    }

    template <typename Self>
    static constexpr auto distance(Self& self, cursor_t<Self> const& from,
                                   cursor_t<Self> const& to)
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return std::min({flux::distance(std::get<I>(self.bases_), std::get<I>(from), std::get<I>(to))...});
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto last(Self& self)
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
            && (sized_sequence<const_like_t<Self, Bases>> && ...)
    {
        auto cur = first(self);
        return inc(self, cur, size(self));
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        requires (sized_sequence<const_like_t<Self, Bases>> && ...)
    {
        return std::apply([&](auto&... args) {
            return std::min({flux::size(args)...});
        }, self.bases_);
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return tuple_t<decltype(move_at_<I>(self, cur))...>{
                move_at_<I>(self, cur)...
            };
        }(std::index_sequence_for<Bases...>{});
    }
};

inline constexpr auto zip = detail::zip_fn{};

} // namespace flux

#endif // FLUX_OP_ZIP_HPP_INCLUDED
