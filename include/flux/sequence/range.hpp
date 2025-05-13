
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SEQUENCE_RANGE_HPP_INCLUDED
#define FLUX_SEQUENCE_RANGE_HPP_INCLUDED

#include <flux/core/concepts.hpp>

#include <ranges>

namespace flux {

namespace detail {

template <typename R>
concept can_const_iterate =
    std::ranges::input_range<R> && std::ranges::input_range<R const> &&
    std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<R const>>;

template <typename R, bool IsConst>
struct range_sequence : inline_sequence_base<range_sequence<R, IsConst>> {
private:
    R rng_;

    using V = std::conditional_t<IsConst, R const, R>;

public:
    struct flux_sequence_traits : default_sequence_traits {
    private:
        class cursor_type {

            std::ranges::iterator_t<V> iter;

            friend struct flux_sequence_traits;

            constexpr explicit cursor_type(std::ranges::iterator_t<V> iter) : iter(std::move(iter)) {}
        public:
            cursor_type() = default;
            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;
            cursor_type(cursor_type const&) requires std::ranges::forward_range<V>
                = default;
            cursor_type& operator=(cursor_type const&) requires std::ranges::forward_range<V>
                = default;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool = default;

            friend auto operator<=>(cursor_type const& lhs, cursor_type const& rhs)
                -> std::strong_ordering
                = default;

        };

    public:
        using value_type = std::ranges::range_value_t<R>;

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto first(Self& self) -> cursor_type
        {
            if constexpr (IsConst) {
                return cursor_type{std::ranges::cbegin(self.rng_)};
            } else {
                return cursor_type{std::ranges::begin(self.rng_)};
            }
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            if constexpr (IsConst) {
                return cur.iter == std::ranges::cend(self.rng_);
            } else {
                return cur.iter == std::ranges::end(self.rng_);
            }
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto inc(Self& self, cursor_type& cur)
        {
            bounds_check(!is_last(self, cur));
            ++cur.iter;
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto read_at(Self& self, cursor_type const& cur) -> decltype(auto)
        {
            bounds_check(!is_last(self, cur));
            return *cur.iter;
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto move_at(Self& self, cursor_type const& cur) -> decltype(auto)
        {
            bounds_check(!is_last(self, cur));
            return std::ranges::iter_move(cur.iter);
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto dec(Self& self, cursor_type& cur)
            requires std::ranges::bidirectional_range<R>
        {
            bounds_check(cur != first(self));
            --cur.iter;
        }

        template <typename Self>
            requires(!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto inc(Self& self, cursor_type& cur, int_t offset)
            requires std::ranges::random_access_range<R>
        {
            if (offset < 0) {
                bounds_check(num::add(offset, distance(self, first(self), cur)) >= 0);
            } else if (offset > 0) {
                bounds_check(offset <= distance(self, cur, last(self)));
            }

            cur.iter += num::cast<std::ranges::range_difference_t<V>>(offset);
        }

        template <typename Self>
            requires(!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto distance(Self&, cursor_type const& from, cursor_type const& to)
            -> int_t
            requires std::ranges::random_access_range<R>
        {
            return num::cast<int_t>(std::ranges::distance(from.iter, to.iter));
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto data(Self& self)
            requires std::ranges::contiguous_range<R>
        {
            if constexpr (IsConst) {
                return std::ranges::cdata(self.rng_);
            } else {
                return std::ranges::data(self.rng_);
            }
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto last(Self& self) -> cursor_type
            requires std::ranges::common_range<R>
        {
            return cursor_type{std::ranges::end(self.rng_)};
        }

        template <typename Self>
            requires(!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto size(Self& self) -> int_t
            requires std::ranges::sized_range<R>
        {
            return num::cast<int_t>(std::ranges::ssize(self.rng_));
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto for_each_while(Self& self, auto&& pred) -> cursor_type
        {
            auto iter = std::ranges::begin(self.rng_);
            auto const end = std::ranges::end(self.rng_);

            while (iter != end) {
                if (!std::invoke(pred, *iter)) {
                    break;
                }
                ++iter;
            }
            return cursor_type{std::move(iter)};
        }
    };

    constexpr explicit range_sequence(R rng) : rng_(std::move(rng)) {}

    constexpr auto begin()
    {
        if constexpr (IsConst) {
            return std::ranges::cbegin(rng_);
        } else {
            return std::ranges::begin(rng_);
        }
    }

    constexpr auto begin() const requires std::ranges::range<R const>
    {
        return std::ranges::begin(rng_);
    }

    constexpr auto end()
    {
        if constexpr (IsConst) {
            return std::ranges::cend(rng_);
        } else {
            return std::ranges::end(rng_);
        }
    }

    constexpr auto end() const requires std::ranges::range<R const>
    {
        return std::ranges::end(rng_);
    }
};

struct from_range_fn {
    template <std::ranges::viewable_range R>
    requires std::ranges::input_range<R>
    constexpr auto operator()(R&& rng) const
    {
        return range_sequence<std::views::all_t<R>, false>(std::views::all(FLUX_FWD(rng)));
    }
};

struct from_crange_fn {
    template <typename R, typename C = std::remove_reference_t<R> const&>
        requires std::ranges::input_range<C> &&
                 std::ranges::viewable_range<C>
    constexpr auto operator()(R&& rng) const
    {
        if constexpr (std::is_lvalue_reference_v<R>) {
            return range_sequence<std::views::all_t<C>, true>(std::views::all(static_cast<C>(rng)));
        } else {
            return range_sequence<std::views::all_t<R>, true>(std::views::all(FLUX_FWD(rng)));
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto from_range = detail::from_range_fn{};
FLUX_EXPORT inline constexpr auto from_crange = detail::from_crange_fn{};

} // namespace flux

#endif // FLUX_SEQUENCE_RANGE_HPP_INCLUDED
