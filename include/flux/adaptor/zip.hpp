
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_ZIP_HPP_INCLUDED
#define FLUX_ADAPTOR_ZIP_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/sequence/empty.hpp>

#include <algorithm> // for std::min({ilist...})

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

}

template <typename... Bases>
struct zip_traits_base : default_iter_traits {
private:
    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

protected:

    template <typename... Ts>
    using tuple_t = detail::pair_or_tuple_t<Ts...>;

    template <std::size_t I>
    static constexpr decltype(auto) read1_(auto fn, auto& self, auto const& cur)
    {
        return fn(std::get<I>(self.bases_), std::get<I>(cur));
    }

public:

    static constexpr bool is_infinite = (infinite_sequence<Bases> && ...);

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto first(Self& self)
    {
        return std::apply([](auto&&... args) {
            return tuple_t<decltype(flux::first(FLUX_FWD(args)))...>(flux::first(FLUX_FWD(args))...);
        }, self.bases_);
    }

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr bool is_last(Self& self, cursor_t<Self> const& cur)
    {
        return [&self, &cur]<std::size_t... I>(std::index_sequence<I...>) {
            return (flux::is_last(std::get<I>(self.bases_), std::get<I>(cur)) || ...);
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto& inc(Self& self, cursor_t<Self>& cur)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::inc(std::get<I>(self.bases_), std::get<I>(cur)), ...);
        }(std::index_sequence_for<Bases...>{});

        return cur;
    }

    template <typename Self>
        requires (bidirectional_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto& dec(Self& self, cursor_t<Self>& cur)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::dec(std::get<I>(self.bases_), std::get<I>(cur)), ...);
        }(std::index_sequence_for<Bases...>{});

        return cur;
    }

    template <typename Self>
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto& inc(Self& self, cursor_t<Self>& cur, distance_t offset)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::inc(std::get<I>(self.bases_), std::get<I>(cur), offset), ...);
        }(std::index_sequence_for<Bases...>{});

        return cur;
    }

    template <typename Self>
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto distance(Self& self, cursor_t<Self> const& from,
                                   cursor_t<Self> const& to)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return std::min({flux::distance(std::get<I>(self.bases_), std::get<I>(from), std::get<I>(to))...});
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
                && (sized_iterable<const_like_t<Self, Bases>> && ...)
    static constexpr auto last(Self& self)
    {
        auto cur = first(self);
        return inc(self, cur, size(self));
    }

    template <typename Self>
        requires (sized_iterable<const_like_t<Self, Bases>> && ...)
    static constexpr auto size(Self& self)
    {
        return std::apply([&](auto&... args) {
            return std::min({flux::size(args)...});
        }, self.bases_);
    }
};


namespace detail {

template <sequence... Bases>
struct zip_adaptor : inline_sequence_base<zip_adaptor<Bases...>> {
private:
    pair_or_tuple_t<Bases...> bases_;

    friend struct iter_traits<zip_adaptor>;
    friend struct zip_traits_base<Bases...>;

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

template <typename Func, sequence... Bases>
struct zip_map_adaptor : inline_sequence_base<zip_map_adaptor<Func, Bases...>> {
private:
    pair_or_tuple_t<Bases...> bases_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

    friend struct iter_traits<zip_map_adaptor>;
    friend struct zip_traits_base<Bases...>;

public:
    constexpr explicit zip_map_adaptor(Func&& func, decays_to<Bases> auto&&... bases)
        : bases_(FLUX_FWD(bases)...), func_(std::move(func))
    {}
};

struct zip_map_fn {
    template <typename Func, adaptable_sequence... Seqs>
        requires std::regular_invocable<Func&, element_t<Seqs>...>
    [[nodiscard]]
    constexpr auto operator()(Func func, Seqs&&... seqs) const
    {
        if constexpr (sizeof...(Seqs) == 0) {
            return empty<std::invoke_result_t<Func>>;
        } else {
            return zip_map_adaptor<Func, std::decay_t<Seqs>...>(std::move(func), FLUX_FWD(seqs)...);
        }
    }
};

} // namespace detail

template <typename... Bases>
struct iter_traits<detail::zip_adaptor<Bases...>> : zip_traits_base<Bases...>
{
private:
    using base = zip_traits_base<Bases...>;

    template <typename... Ts>
    using tuple_t = base::template tuple_t<Ts...>;

    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    static constexpr auto read_(auto fn, auto& self, auto const& cur)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
          return tuple_t<decltype(base::template read1_<I>(fn, self, cur))...> {
              base::template read1_<I>(fn, self, cur)...
          };
        }(std::index_sequence_for<Bases...>{});
    }

public:
    using value_type = tuple_t<value_t<Bases>...>;

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::read_at, self, cur);
    }

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::move_at, self, cur);
    }

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto read_at_unchecked(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::read_at_unchecked, self, cur);
    }

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto move_at_unchecked(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::move_at_unchecked, self, cur);
    }

};

template <typename Func, typename... Bases>
struct iter_traits<detail::zip_map_adaptor<Func, Bases...>> : zip_traits_base<Bases...>
{
private:
    using base = zip_traits_base<Bases...>;

    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    static constexpr decltype(auto) read_(auto fn, auto& self, auto const& cur)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) -> decltype(auto) {
            return std::invoke(self.func_,
                base::template read1_<I>(fn, self, cur)...
            );
        }(std::index_sequence_for<Bases...>{});
    }

public:
    using value_type = std::remove_cvref_t<std::invoke_result_t<Func&, element_t<Bases>...>>;

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr decltype(auto) read_at(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::read_at, self, cur);
    }

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr decltype(auto) read_at_unchecked(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::read_at_unchecked, self, cur);
    }

    using default_iter_traits::move_at;
    using default_iter_traits::move_at_unchecked;
};

FLUX_EXPORT inline constexpr auto zip = detail::zip_fn{};
FLUX_EXPORT inline constexpr auto zip_map = detail::zip_map_fn{};

} // namespace flux

#endif // FLUX_ADAPTOR_ZIP_HPP_INCLUDED
