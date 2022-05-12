
#pragma once

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

template <lens... Bases>
struct zip_adaptor : lens_base<zip_adaptor<Bases...>> {
private:
    pair_or_tuple_t<Bases...> bases_;

    friend struct sequence_iface<zip_adaptor>;

public:
    constexpr explicit zip_adaptor(Bases&&... bases)
        : bases_(std::move(bases)...)
    {}
};

struct zip_fn {
    template <sequence... Seqs>
    constexpr auto operator()(Seqs&&... seqs) const
    {
        if constexpr (sizeof...(Seqs) == 0) {
            return empty<std::tuple<>>;
        } else {
            return zip_adaptor(flux::from(FLUX_FWD(seqs))...);
        }
    }
};

} // namespace detail

template <typename... Bases>
struct sequence_iface<detail::zip_adaptor<Bases...>>
{
private:
    template <typename... Ts>
    using tuple_t = detail::pair_or_tuple_t<Ts...>;

    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    template <std::size_t I>
    static constexpr decltype(auto) read_at_(auto& self, auto const& idx)
    {
        return flux::read_at(std::get<I>(self.bases_), std::get<I>(idx));
    }

    template <std::size_t I>
    static constexpr decltype(auto) move_at_(auto& self, auto const& idx)
    {
        return flux::move_at(std::get<I>(self.bases_), std::get<I>(idx));
    }

public:
    using value_type = tuple_t<value_t<Bases>...>;
    using distance_type = std::common_type_t<distance_t<Bases>...>;

    static constexpr bool is_infinite = (infinite_sequence<Bases> && ...);

    static constexpr auto first(auto& self)
    {
        return std::apply([](auto&&... args) {
            return tuple_t<decltype(flux::first(FLUX_FWD(args)))...>(flux::first(FLUX_FWD(args))...);
        }, self.bases_);
    }

    template <typename Self>
    static constexpr bool is_last(Self& self, index_t<Self> const& idx)
    {
        return [&self, &idx]<std::size_t... I>(std::index_sequence<I...>) {
            return (flux::is_last(std::get<I>(self.bases_), std::get<I>(idx)) || ...);
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, index_t<Self> const& idx)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return tuple_t<decltype(read_at_<I>(self, idx))...> {
                read_at_<I>(self, idx)...
            };
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto& inc(Self& self, index_t<Self>& idx)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::inc(std::get<I>(self.bases_), std::get<I>(idx)), ...);
        }(std::index_sequence_for<Bases...>{});

        return idx;
    }

    template <typename Self>
    static constexpr auto& dec(Self& self, index_t<Self>& idx)
        requires (bidirectional_sequence<const_like_t<Self, Bases>> && ...)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::dec(std::get<I>(self.bases_), std::get<I>(idx)), ...);
        }(std::index_sequence_for<Bases...>{});

        return idx;
    }

    template <typename Self>
    static constexpr auto& inc(Self& self, index_t<Self>& idx, distance_t<Self> offset)
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::inc(std::get<I>(self.bases_), std::get<I>(idx), offset), ...);
        }(std::index_sequence_for<Bases...>{});

        return idx;
    }

    template <typename Self>
    static constexpr auto distance(Self& self, index_t<Self> const& from, index_t<Self> const& to)
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return std::min({
                static_cast<distance_t<Self>>(
                    flux::distance(std::get<I>(self.bases_), std::get<I>(from), std::get<I>(to)))...});
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto last(Self& self)
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
            && (sized_sequence<const_like_t<Self, Bases>> && ...)
    {
        auto idx = first(self);
        return inc(self, idx, size(self));
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        requires (sized_sequence<const_like_t<Self, Bases>> && ...)
    {
        return std::apply([&](auto&... args) {
            return std::min({static_cast<distance_t<Self>>(flux::size(args))...});
        }, self.bases_);
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, index_t<Self> const& idx)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return tuple_t<decltype(move_at_<I>(self, idx))...>{
                move_at_<I>(self, idx)...
            };
        }(std::index_sequence_for<Bases...>{});
    }
};

inline constexpr auto zip = detail::zip_fn{};

} // namespace flux