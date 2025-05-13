
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_ADJACENT_HPP_INCLUDED
#define FLUX_ADAPTOR_ADJACENT_HPP_INCLUDED

#include <flux/core.hpp>

#include <flux/adaptor/reverse.hpp>
#include <flux/adaptor/zip.hpp>
#include <flux/sequence/iota.hpp>

#include <array>


namespace flux {

namespace detail {

template <typename Base, int_t N>
struct adjacent_sequence_traits_base : default_sequence_traits {
protected:
    struct cursor_type {
        std::array<cursor_t<Base>, N> arr{};

        friend constexpr auto operator==(cursor_type const& lhs, cursor_type const& rhs)
            -> bool
        {
            return lhs.arr.back() == rhs.arr.back();
        }

        friend constexpr auto operator<=>(cursor_type const& lhs, cursor_type const& rhs)
            -> std::strong_ordering
            requires ordered_cursor<cursor_t<Base>>
        {
            return lhs.arr.back() <=> rhs.arr.back();
        }
    };

public:

    static inline constexpr bool is_infinite = infinite_sequence<Base>;

    static constexpr auto first(auto& self) -> cursor_type
    {
        cursor_type out{flux::first(self.base_), };

        FLUX_FOR(auto i, flux::iota(std::size_t{1}, std::size_t{N})) {
            out.arr[i] = out.arr[i - 1];
            if (!flux::is_last(self.base_, out.arr[i])) {
                flux::inc(self.base_, out.arr[i]);
            }
        }
        return out;
    }

    static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
    {
        return flux::is_last(self.base_, cur.arr.back());
    }

    static constexpr auto inc(auto& self, cursor_type& cur) -> void
    {
        std::apply([&](auto&... curs) {
            (flux::inc(self.base_, curs), ...);
        }, cur.arr);
    }

    static constexpr auto last(auto& self) -> cursor_type
        requires (bidirectional_sequence<Base> && bounded_sequence<Base>)
    {
        cursor_type out{};
        out.arr.back() = flux::last(self.base_);
        auto const first = flux::first(self.base_);
        FLUX_FOR(auto i, flux::iota(std::size_t{0}, std::size_t{N}-1).reverse()) {
            out.arr[i] = out.arr[i + 1];
            if (out.arr[i] != first) {
                flux::dec(self.base_, out.arr[i]);
            }
        }
        return out;
    }

    static constexpr auto dec(auto& self, cursor_type& cur) -> void
        requires bidirectional_sequence<Base>
    {
        std::apply([&self](auto&... curs) {
            (flux::dec(self.base_, curs), ...);
        }, cur.arr);
    }

    static constexpr auto inc(auto& self, cursor_type& cur, int_t offset) -> void
        requires random_access_sequence<Base>
    {
        std::apply([&self, offset](auto&... curs) {
            (flux::inc(self.base_, curs, offset), ...);
        }, cur.arr);
    }

    static constexpr auto distance(auto& self, cursor_type const& from, cursor_type const& to)
        -> int_t
        requires random_access_sequence<Base>
    {
        return flux::distance(self.base_, from.arr.back(), to.arr.back());
    }

    static constexpr auto size(auto& self) -> int_t
        requires sized_sequence<Base>
    {
        auto s = (flux::size(self.base_) - N) + 1;
        return (cmp::max)(s, int_t {0});
    }
};

template <typename Base, int_t N>
struct adjacent_adaptor : inline_sequence_base<adjacent_adaptor<Base, N>> {
private:
    Base base_;

    friend struct adjacent_sequence_traits_base<Base, N>;

public:
    constexpr explicit adjacent_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_sequence_traits : adjacent_sequence_traits_base<Base, N> {
    private:

        using cursor_type = adjacent_sequence_traits_base<Base, N>::cursor_type;

        template <auto& ReadFn>
        static constexpr auto do_read(auto& self, cursor_type const& cur)
        {
            return std::apply([&](auto const&... curs) {
                return pair_or_tuple_t<decltype(ReadFn(self.base_, curs))...>(
                    ReadFn(self.base_, curs)...);
            }, cur.arr);
        }

        template <std::size_t I>
        using base_value_t = value_t<Base>;

        template <std::size_t... Is>
        static auto make_value_type(std::index_sequence<Is...>) -> pair_or_tuple_t<base_value_t<Is>...>;

    public:
        using value_type = decltype(make_value_type(std::make_index_sequence<N>{}));

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(do_read<flux::read_at>(self, cur))
        {
            return do_read<flux::read_at>(self, cur);
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(do_read<flux::move_at>( self, cur))
        {
            return do_read<flux::move_at>( self, cur);
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(do_read<flux::read_at_unchecked>(self, cur))
        {
            return do_read<flux::read_at_unchecked>(self, cur);
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(do_read<flux::move_at_unchecked>(self, cur))
        {
            return do_read<flux::move_at_unchecked>(self, cur);
        }
    };
};

template <typename Base, int_t N, typename Func>
struct adjacent_map_adaptor : inline_sequence_base<adjacent_map_adaptor<Base, N, Func>> {
private:
    Base base_;
    Func func_;

    friend struct adjacent_sequence_traits_base<Base, N>;

public:
    constexpr explicit adjacent_map_adaptor(decays_to<Base> auto&& base, Func&& func)
        : base_(FLUX_FWD(base)),
          func_(std::move(func))
    {}

    struct flux_sequence_traits : adjacent_sequence_traits_base<Base, N> {
        template <typename Self>
        static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
            -> decltype(auto)
            requires repeated_invocable<decltype((self.func_)), element_t<decltype((self.base_))>, N>
        {
            return std::apply([&](auto const&... curs) {
                return std::invoke(self.func_, flux::read_at(self.base_, curs)...);
            }, cur.arr);
        }
    };
};

template <int_t N>
struct adjacent_fn {
    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> multipass_sequence auto
    {
        return adjacent_adaptor<std::decay_t<Seq>, N>(FLUX_FWD(seq));
    }
};

template <int_t N>
struct adjacent_map_fn {
    template <adaptable_sequence Seq, typename Func>
        requires multipass_sequence<Seq> &&
                 repeated_invocable<Func, element_t<Seq>, N>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func) const -> multipass_sequence auto
    {
        return adjacent_map_adaptor<std::decay_t<Seq>, N, Func>(
            FLUX_FWD(seq), std::move(func));
    }
};

} // namespace detail

FLUX_EXPORT
template <int_t N>
    requires(N > 0)
inline constexpr auto adjacent = detail::adjacent_fn<N> {};

FLUX_EXPORT inline constexpr auto pairwise = adjacent<2>;

FLUX_EXPORT
template <int_t N>
    requires(N > 0)
inline constexpr auto adjacent_map = detail::adjacent_map_fn<N> {};

FLUX_EXPORT inline constexpr auto pairwise_map = adjacent_map<2>;

template <typename D>
template <int_t N>
constexpr auto inline_sequence_base<D>::adjacent() &&
    requires multipass_sequence<D>
{
    return flux::adjacent<N>(std::move(derived()));
}

template <typename D>
constexpr auto inline_sequence_base<D>::pairwise() &&
    requires multipass_sequence<D>
{
    return flux::pairwise(std::move(derived()));
}

template <typename D>
template <int_t N, typename Func>
    requires multipass_sequence<D>
constexpr auto inline_sequence_base<D>::adjacent_map(Func func) &&
{
    return flux::adjacent_map<N>(std::move(derived()), std::move(func));
}

template <typename D>
template <typename Func>
    requires multipass_sequence<D>
constexpr auto inline_sequence_base<D>::pairwise_map(Func func) &&
{
    return flux::pairwise_map(std::move(derived()), std::move(func));
}

} // namespace flux

#endif // FLUX_ADAPTOR_ADJACENT_HPP_INCLUDED
