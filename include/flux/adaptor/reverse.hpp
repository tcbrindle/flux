
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_REVERSE_HPP_INCLUDED
#define FLUX_ADAPTOR_REVERSE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <reverse_iterable Base>
struct reverse_adaptor : inline_sequence_base<reverse_adaptor<Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;

public:
    constexpr explicit reverse_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base&& { return std::move(base_); }

    struct flux_iterable_traits {
        static constexpr auto iterate(auto& self)
            requires reverse_iterable<decltype((self.base_))>
        {
            return flux::reverse_iterate(self.base_);
        }

        static constexpr auto reverse_iterate(auto& self)
            requires iterable<decltype((self.base_))>
        {
            return flux::iterate(self.base_);
        }

        static constexpr auto size(auto& self)
            requires sized_iterable<decltype((self.base_))>
        {
            return flux::iterable_size(self.base_);
        }
    };

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

        template <typename Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur)
            -> element_t<decltype((self.base_))>
        {
            return flux::read_at(self.base_, flux::prev(self.base_, cur.base_cur));
        }

        template <typename Self>
        static constexpr auto read_at_unchecked(Self& self, cursor_type const& cur)
            -> element_t<decltype((self.base_))>
        {
            return flux::read_at_unchecked(self.base_, flux::prev(self.base_, cur.base_cur));
        }

        template <typename Self>
        static constexpr auto move_at(Self& self, cursor_type const& cur)
            -> rvalue_element_t<decltype((self.base_))>
        {
            return flux::move_at(self.base_, flux::prev(self.base_, cur.base_cur));
        }

        template <typename Self>
        static constexpr auto move_at_unchecked(Self& self, cursor_type const& cur)
            -> rvalue_element_t<decltype((self.base_))>
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

        static constexpr auto inc(auto& self, cursor_type& cur, int_t dist) -> void
        {
            flux::inc(self.base_, cur.base_cur, num::neg(dist));
        }

        static constexpr auto distance(auto& self, cursor_type const& from, cursor_type const& to)
            -> int_t
        {
            return flux::distance(self.base_, to.base_cur, from.base_cur);
        }

        static constexpr auto size(auto& self) -> int_t
            requires sized_sequence<decltype((self.base_))>
        {
            return flux::size(self.base_);
        }

        static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_type
            requires bidirectional_sequence<decltype((self.base_))> &&
                     bounded_sequence<decltype((self.base_))>
        {
            auto cur = flux::last(self.base_);
            const auto end = flux::first(self.base_);

            while (cur != end) {
                flux::dec(self.base_, cur);
                if (!std::invoke(pred, flux::read_at(self.base_, cur))) {
                    flux::inc(self.base_, cur);
                    break;
                }
            }

            return cursor_type(cur);
        }
    };
};

template <typename>
inline constexpr bool is_reverse_adaptor = false;

template <typename Base>
inline constexpr bool is_reverse_adaptor<reverse_adaptor<Base>> = true;

struct reverse_fn {
    template <adaptable_iterable It>
        requires reverse_iterable<It>
    [[nodiscard]]
    constexpr auto operator()(It&& it) const -> reverse_iterable auto
    {
        if constexpr (is_reverse_adaptor<std::decay_t<It>>) {
            return FLUX_FWD(it).base();
        } else {
            return reverse_adaptor<std::decay_t<It>>(FLUX_FWD(it));
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

#endif // FLUX_ADAPTOR_REVERSE_HPP_INCLUDED
