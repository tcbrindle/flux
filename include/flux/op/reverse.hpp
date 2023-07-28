
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_REVERSE_HPP_INCLUDED
#define FLUX_OP_REVERSE_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>

namespace flux {

namespace detail {

template <bidirectional_sequence Base>
    requires bounded_sequence<Base>
struct reverse_adaptor : inline_sequence_base<reverse_adaptor<Base>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;

public:
    constexpr explicit reverse_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base&& { return std::move(base_); }

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;

            friend bool operator==(cursor_type const&, cursor_type const&)
                requires std::equality_comparable<cursor_t<Base>>
            = default;

            friend auto operator<=>(cursor_type const& lhs, cursor_type const& rhs)
                -> std::strong_ordering
                requires std::three_way_comparable<cursor_t<Base>, std::strong_ordering>
            {
                return rhs <=> lhs;
            }
        };

    public:
        using value_type = value_t<Base>;

        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type(flux::last(self.base_));
        }

        static constexpr auto last(auto& self) -> cursor_type
        {
            return cursor_type(flux::first(self.base_));
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return cur.base_cur == flux::first(self.base_);
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur) -> decltype(auto)
        {
            return flux::read_at(self.base_, flux::prev(self.base_, cur.base_cur));
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur) -> decltype(auto)
        {
            return flux::read_at_unchecked(self.base_, flux::prev(self.base_, cur.base_cur));
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur) -> decltype(auto)
        {
            return flux::move_at(self.base_, flux::prev(self.base_, cur.base_cur));
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur) -> decltype(auto)
        {
            return flux::move_at_unchecked(self.base_, flux::prev(self.base_, cur.base_cur));
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            flux::dec(self.base_, cur.base_cur);
        }


        static constexpr auto dec(auto& self, cursor_type& cur) -> void
        {
            flux::inc(self.base_, cur.base_cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur, distance_t dist) -> void
            requires random_access_sequence<decltype(self.base_)>
        {
            flux::inc(self.base_, cur.base_cur, -dist);
        }

        static constexpr auto distance(auto& self, cursor_type const& from, cursor_type const& to)
            -> distance_t
            requires random_access_sequence<decltype(self.base_)>
        {
            return flux::distance(self.base_, to.base_cur, from.base_cur);
        }

        static constexpr auto size(auto& self) -> distance_t
            requires sized_sequence<decltype(self.base_)>
        {
            return flux::size(self.base_);
        }

        static constexpr auto for_each_while(auto& self, auto&& pred)
        {
            auto cur = flux::last(self.base_);
            const auto end = flux::first(self.base_);

            while (cur != end) {
                flux::dec(self.base_, cur);
                if (!std::invoke(pred, flux::read_at(self.base_, cur))) {
                    break;
                }
            }

            return cursor_type(flux::inc(self.base_, cur));
        }
    };
};

template <typename>
inline constexpr bool is_reverse_adaptor = false;

template <typename Base>
inline constexpr bool is_reverse_adaptor<reverse_adaptor<Base>> = true;

struct reverse_fn {
    template <adaptable_sequence Seq>
        requires bidirectional_sequence<Seq> &&
                 bounded_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
        -> sequence auto
    {
        if constexpr (is_reverse_adaptor<std::decay_t<Seq>>) {
            return FLUX_FWD(seq).base();
        } else {
            return reverse_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto reverse = detail::reverse_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::reverse() &&
    requires bidirectional_sequence<D> && bounded_sequence<D>
{
    return flux::reverse(std::move(derived()));
}

} // namespace flux

#endif
