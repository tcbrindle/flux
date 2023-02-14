
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ADJACENT_HPP_INCLUDED
#define FLUX_OP_ADJACENT_HPP_INCLUDED

#include <flux/core.hpp>

#include <flux/op/begin_end.hpp>
#include <flux/op/reverse.hpp>
#include <flux/op/zip.hpp>
#include <flux/source/iota.hpp>


namespace flux {

namespace detail {

template <typename Base, distance_t N>
struct adjacent_adaptor : inline_sequence_base<adjacent_adaptor<Base, N>> {
private:
    Base base_;

public:
    constexpr explicit adjacent_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_sequence_traits {
    private:
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

        static constexpr auto do_read(auto const& fn, auto& self, cursor_type const& cur)
        {
            return std::apply([&](auto const&... curs) {
                return pair_or_tuple_t<decltype(fn(self.base_, curs))...>(
                    fn(self.base_, curs)...);
            }, cur.arr);
        }

        template <std::size_t I>
        using base_value_t = value_t<Base>;

        template <std::size_t... Is>
        static auto make_value_type(std::index_sequence<Is...>) -> pair_or_tuple_t<base_value_t<Is>...>;

    public:

        static inline constexpr bool is_infinite = infinite_sequence<Base>;

        using value_type = decltype(make_value_type(std::make_index_sequence<N>{}));

        static constexpr auto first(auto& self) -> cursor_type
        {
            cursor_type out{flux::first(self.base_), };

            for (auto i : flux::ints(1, N)) {
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

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(do_read(flux::read_at, self, cur))
        {
            return do_read(flux::read_at, self, cur);
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(do_read(flux::move_at, self, cur))
        {
            return do_read(flux::move_at, self, cur);
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(do_read(flux::read_at_unchecked, self, cur))
        {
            return do_read(flux::read_at_unchecked, self, cur);
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(do_read(flux::move_at_unchecked, self, cur))
        {
            return do_read(flux::move_at_unchecked, self, cur);
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires (bidirectional_sequence<Base> && bounded_sequence<Base>)
        {
            cursor_type out{};
            out.arr.back() = flux::last(self.base_);
            for (auto i : flux::ints(0, N-1).reverse()) {
                out.arr[i] = flux::prev(self.base_, out.arr[i+1]);
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
    };
};

template <distance_t N>
struct adjacent_fn {
    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> multipass_sequence auto
    {
        return adjacent_adaptor<std::decay_t<Seq>, N>(FLUX_FWD(seq));
    }
};

} // namespace detail

template <distance_t N>
    requires (N > 0)
inline constexpr auto adjacent = detail::adjacent_fn<N>{};

inline constexpr auto pairwise = adjacent<2>;

template <typename D>
template <distance_t N>
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

} // namespace flux

#endif // FLUX_OP_ADJACENT_HPP_INCLUDED
