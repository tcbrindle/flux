
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CYCLE_HPP_INCLUDED
#define FLUX_OP_CYCLE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <multipass_sequence Base>
struct cycle_adaptor : inline_sequence_base<cycle_adaptor<Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;

public:
    constexpr explicit cycle_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;
            // Use an unsigned type to avoid UB on overflow
            std::size_t n = 0;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool = default;

            friend auto operator<=>(cursor_type const&, cursor_type const&)
                -> std::strong_ordering
                requires std::three_way_comparable<cursor_t<Base>, std::strong_ordering>
            = default;
        };

    public:
        using value_type = value_t<Base>;

        static constexpr bool is_infinite = true;

        static constexpr auto first(auto& self)
            -> decltype(cursor_type{flux::first(self.base_)})
        {
            return cursor_type{flux::first(self.base_)};
        }

        static constexpr auto is_last(auto&, cursor_type const&) -> bool { return false; }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            flux::inc(self.base_, cur.base_cur);
            if (flux::is_last(self.base_, cur.base_cur)) {
                cur.base_cur = flux::first(self.base_);
                ++cur.n;
            }
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(static_cast<const_element_t<Base>>(flux::read_at(self.base_, cur.base_cur)))
        {
            return static_cast<const_element_t<Base>>(
                flux::read_at(self.base_, cur.base_cur));
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> const_element_t<Base>
        {
            return static_cast<const_element_t<Base>>(
                flux::read_at_unchecked(self.base_, cur.base_cur));
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(auto)
        {
            using R = std::common_reference_t<value_t<Base> const&&, rvalue_element_t<Base>>;
            return static_cast<R>(flux::move_at(self.base_, cur.base_cur));
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(auto)
        {
            using R = std::common_reference_t<value_t<Base> const&&, rvalue_element_t<Base>>;
            return static_cast<R>(flux::move_at_unchecked(self.base_, cur.base_cur));
        }

        static constexpr auto for_each_while(auto& self, auto&& func) -> cursor_type
        {
            std::size_t n = 0;
            while (true) {
                auto cur = flux::for_each_while(self.base_, std::ref(func));
                if (!flux::is_last(self.base_, cur)) {
                    return cursor_type{std::move(cur), n};
                }
                ++n;
            }
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
            requires bidirectional_sequence<decltype(self.base_)> &&
                     bounded_sequence<decltype(self.base_)>
        {
            if (cur.base_cur == flux::first(self.base_)) {
                --cur.n;
                cur.base_cur = flux::last(self.base_);
            }
            flux::dec(self.base_, cur.base_cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur, distance_t offset)
            requires random_access_sequence<decltype(self.base_)> &&
                     bounded_sequence<decltype(self.base_)>
        {
            auto const first = flux::first(self.base_);

            auto const sz = flux::size(self.base_);
            if (sz == 0) {
                return;
            }

            auto off = flux::distance(self.base_, first, cur.base_cur);
            off = num::checked_add(off, offset);

            cur.n += off/sz;
            FLUX_DEBUG_ASSERT(cur.n >= 0);

            off = off % sz;
            if (off < 0) {
                off +=sz; // differing signs
            }

            cur.base_cur = flux::next(self.base_, first, off);
        }

        static constexpr auto distance(auto& self,
                                       cursor_type const& from,
                                       cursor_type const& to) -> distance_t
            requires random_access_sequence<decltype(self.base_)> &&
                     sized_sequence<decltype(self.base_)>
        {
            auto dist = checked_cast<distance_t>(to.n) - checked_cast<distance_t>(from.n);
            dist = num::checked_mul(dist, flux::size(self.base_));
            return num::checked_add(dist,
                    flux::distance(self.base_, from.base_cur, to.base_cur));
        }
    };
};

struct cycle_fn {
    template <adaptable_sequence Seq>
        requires infinite_sequence<Seq> || multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> infinite_sequence auto
    {
        if constexpr (infinite_sequence<Seq>) {
            return FLUX_FWD(seq);
        } else {
            return cycle_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
        }
    }
};

} // namespace detail

inline constexpr auto cycle = detail::cycle_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::cycle() &&
    requires infinite_sequence<D> || multipass_sequence<D>
{
    return flux::cycle(std::move(derived()));
}

} // namespace flux

#endif // FLUX_OP_CYCLE_HPP_INCLUDED
