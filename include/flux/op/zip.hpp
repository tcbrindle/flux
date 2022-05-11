
#pragma once

#include <flux/core.hpp>
#include <flux/source/empty.hpp>

namespace flux {

namespace detail {


template <lens... Bases>
struct zip_adaptor : lens_base<zip_adaptor<Bases...>> {
private:
    std::tuple<Bases...> bases_;

    friend struct sequence_iface<zip_adaptor>;

public:
    constexpr explicit zip_adaptor(Bases&&... bases)
        : bases_(std::move(bases)...)
    {}
};

template <typename Self, typename... Bases>
inline constexpr bool all_bidirectional =
    std::is_const_v<Self>
        ? (bidirectional_sequence<Bases const> && ...)
        : (bidirectional_sequence<Bases> && ...);

template <typename Self, typename... Bases>
inline constexpr bool all_random_access =
    std::is_const_v<Self>
        ? (random_access_sequence<Bases const> && ...)
        : (random_access_sequence<Bases> && ...);

template <typename Self, typename... Bases>
inline constexpr bool all_sized =
    std::is_const_v<Self>
        ? (sized_sequence<Bases const> && ...)
        : (sized_sequence<Bases> && ...);

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
    using value_type = std::tuple<value_t<Bases>...>;
    using distance_type = std::common_type_t<distance_t<Bases>...>;

    static constexpr bool is_infinite = (infinite_sequence<Bases> && ...);

    static constexpr auto first(auto& self)
    {
        return std::apply([](auto&&... args) {
            return std::tuple(flux::first(FLUX_FWD(args))...);
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
            return std::tuple<decltype(read_at_<I>(self, idx))...> {
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
        requires detail::all_bidirectional<Self, Bases...>
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::dec(std::get<I>(self.bases_), std::get<I>(idx)), ...);
        }(std::index_sequence_for<Bases...>{});

        return idx;
    }

    template <typename Self>
    static constexpr auto& inc(Self& self, index_t<Self>& idx, distance_t<Self> offset)
        requires detail::all_random_access<Self, Bases...>
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::inc(std::get<I>(self.bases_), std::get<I>(idx), offset), ...);
        }(std::index_sequence_for<Bases...>{});

        return idx;
    }

    template <typename Self>
    static constexpr auto distance(Self& self, index_t<Self> const& from, index_t<Self> const& to)
        requires detail::all_random_access<Self, Bases...>
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return std::min({
                static_cast<distance_t<Self>>(
                    flux::distance(std::get<I>(self.bases_), std::get<I>(from), std::get<I>(to)))...});
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto last(Self& self)
        requires detail::all_random_access<Self, Bases...>
            && detail::all_sized<Self, Bases...>
    {
        auto idx = first(self);
        return inc(self, idx, size(self));
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        requires detail::all_sized<Self, Bases...>
    {
        return std::apply([&](auto&... args) {
            return std::min({static_cast<distance_t<Self>>(flux::size(args))...});
        }, self.bases_);
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, index_t<Self> const& idx)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return std::tuple<decltype(move_at_<I>(self, idx))...>{
                move_at_<I>(self, idx)...
            };
        }(std::index_sequence_for<Bases...>{});
    }
};


template <typename Base1, typename Base2>
struct sequence_iface<detail::zip_adaptor<Base1, Base2>>
{
    using value_type = std::pair<value_t<Base1>, value_t<Base2>>;
    using distance_type = std::common_type_t<distance_t<Base1>, distance_t<Base2>>;

    template <typename Self>
    static constexpr decltype(auto) base1(Self& self) { return std::get<0>(self.bases_); }

    template <typename Self>
    static constexpr decltype(auto) base2(Self& self) { return std::get<1>(self.bases_); }

    static constexpr bool disable_multipass =
        !(multipass_sequence<Base1> && multipass_sequence<Base2>);

    static constexpr auto first(auto& self)
    {
        return std::pair(flux::first(base1(self)), flux::first(base2(self)));
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, index_t<Self> const& idx)
    {
        return flux::is_last(base1(self), idx.first) ||
                flux::is_last(base2(self), idx.second);
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, index_t<Self> const& idx)
    {
        return std::pair<decltype(flux::read_at(base1(self), idx.first)),
                         decltype(flux::read_at(base2(self), idx.second))>(
                    flux::read_at(base1(self), idx.first),
                    flux::read_at(base2(self), idx.second));
    }

    template <typename Self>
    static constexpr auto inc(Self& self, index_t<Self>& idx) -> index_t<Self>&
    {
        flux::inc(base1(self), idx.first);
        flux::inc(base2(self), idx.second);
        return idx;
    }

    template <typename Self>
    static constexpr auto dec(Self& self, index_t<Self>& idx) -> index_t<Self>&
        requires detail::all_bidirectional<Self, Base1, Base2>
    {
        flux::dec(base1(self), idx.first);
        flux::dec(base2(self), idx.second);
        return idx;
    }

    template <typename Self>
    static constexpr auto inc(Self& self, index_t<Self>& idx, distance_type off)
        -> index_t<Self>&
        requires detail::all_random_access<Self, Base1, Base2>
    {
        flux::inc(base1(self), idx.first, off);
        flux::inc(base2(self), idx.second, off);
        return idx;
    }

    template <typename Self>
    static constexpr auto distance(Self& self, index_t<Self> const& from, index_t<Self> const& to)
        -> distance_type
        requires detail::all_random_access<Self, Base1, Base2>
    {
        return std::min<distance_type>(
                    flux::distance(base1(self), from.first, to.first),
                    flux::distance(base2(self), from.second, to.second));
    }

    template <typename Self>
    static constexpr auto last(Self& self)
        requires detail::all_random_access<Self, Base1, Base2>
            && detail::all_sized<Self, Base1, Base2>
    {
        auto idx = first(self);
        return inc(self, idx, size(self));
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        requires detail::all_sized<Self, Base1, Base2>
    {
        return std::min<distance_type>(flux::size(base1(self)), flux::size(base2(self)));
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, index_t<Self> const& idx)
    {
        return std::pair<decltype(flux::move_at(base1(self), idx.first)),
                         decltype(flux::move_at(base2(self), idx.second))>(
            flux::move_at(base1(self), idx.first),
            flux::move_at(base2(self), idx.second));
    }
};

inline constexpr auto zip = detail::zip_fn{};

} // namespace flux