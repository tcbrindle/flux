
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_RANGES_FROM_RANGE_HPP_INCLUDED
#define FLUX_RANGES_FROM_RANGE_HPP_INCLUDED

#include <flux/core/concepts.hpp>

#include <ranges>

namespace flux {

namespace detail {

template <typename R>
concept contiguous_and_sized_range = std::ranges::contiguous_range<R> && std::ranges::sized_range<R>;

}

template <typename R>
    requires (!detail::derived_from_inline_sequence_base<R> &&
              std::ranges::input_range<R> &&
              !detail::contiguous_and_sized_range<R>)
struct sequence_traits<R> {

    using value_type = std::ranges::range_value_t<R>;

    static constexpr bool disable_multipass = !std::ranges::forward_range<R>;

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr auto first(Self& self)
    {
        return std::ranges::begin(self);
    }

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr bool is_last(Self& self, cursor_t<Self> const& cur)
    {
        return cur == std::ranges::end(self);
    }

    template <decays_to<R> Self>
    static constexpr auto read_at(Self&, cursor_t<Self> const& cur)
        -> decltype(auto)
    {
        return *cur;
    }

    template <decays_to<R> Self>
    static constexpr auto inc(Self&, cursor_t<Self>& cur)
        -> cursor_t<Self>&
    {
        return ++cur;
    }

    template <decays_to<R> Self>
        requires std::bidirectional_iterator<cursor_t<Self>>
    static constexpr auto dec(Self&, cursor_t<Self>& cur)
        -> cursor_t<Self>&
    {
        return --cur;
    }

    template <decays_to<R> Self>
        requires std::random_access_iterator<cursor_t<Self>>
    static constexpr auto inc(Self&, cursor_t<Self>& cur, distance_t offset)
        -> cursor_t<Self>&
    {
        return cur += narrow_cast<std::ranges::range_difference_t<Self>>(offset);
    }

    template <decays_to<R> Self>
        requires std::random_access_iterator<cursor_t<Self>>
    static constexpr auto distance(Self&, cursor_t<Self> const& from,
                                   cursor_t<Self> const& to)
    {
        return to - from;
    }

    template <decays_to<R> Self>
        requires std::ranges::contiguous_range<Self>
    static constexpr auto data(Self& self)
    {
        return std::ranges::data(self);
    }

    template <decays_to<R> Self>
        requires std::ranges::common_range<Self>
    static constexpr auto last(Self& self) -> cursor_t<Self>
    {
        return std::ranges::end(self);
    }

    template <decays_to<R> Self>
        requires std::ranges::sized_range<Self>
    static constexpr auto size(Self& self)
    {
        return std::ranges::size(self);
    }

    template <decays_to<R> Self>
    static constexpr auto move_at(Self&, cursor_t<Self> const& cur)
        -> decltype(auto)
    {
        return std::ranges::iter_move(cur);
    }

    template <decays_to<R> Self>
    static constexpr auto slice(Self& self, cursor_t<Self> from)
    {
        return std::ranges::subrange(std::move(from), std::ranges::end(self));
    }

    template <decays_to<R> Self>
    static constexpr auto slice(Self&, cursor_t<Self> from, cursor_t<Self> to)
    {
        return std::ranges::subrange(std::move(from), std::move(to));
    }

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr auto for_each_while(Self& self, auto&& pred)
    {
        auto iter = std::ranges::begin(self);
        auto const last = std::ranges::end(self);

        for ( ; iter != last; ++iter) {
            if (!std::invoke(pred, *iter)) {
                break;
            }
        }

        return iter;
    }
};

template <typename R>
    requires (!detail::derived_from_inline_sequence_base<R> && detail::contiguous_and_sized_range<R>)
struct sequence_traits<R> {

    using cursor_type = std::ranges::range_size_t<R>;
    using value_type = std::ranges::range_value_t<R>;

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr auto first(Self&) -> cursor_type
    {
        return 0;
    }

    template <decays_to<R> Self>
        requires std::ranges::sized_range<Self>
    static constexpr auto is_last(Self& self, cursor_type cur) -> bool
    {
        return cur == std::ranges::size(self);
    }

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr auto inc(Self&, cursor_type& cur) -> cursor_type&
    {
        return ++cur;
    }

    template <decays_to<R> Self>
        requires std::ranges::contiguous_range<Self>
    static constexpr auto read_at(Self& self, cursor_type cur) -> decltype(auto)
    {
        return *(std::ranges::data(self) + cur);
    }

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr auto dec(Self&, cursor_type& cur) -> cursor_type&
    {
        return --cur;
    }

    template <decays_to<R> Self>
        requires std::ranges::sized_range<Self>
    static constexpr auto last(Self& self) -> cursor_type
    {
        return std::ranges::size(self);
    }

    template <decays_to<R> Self>
        requires std::ranges::sized_range<Self>
    static constexpr auto size(Self& self) -> std::ranges::range_size_t<R>
    {
        return std::ranges::size(self);
    }

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr auto inc(Self&, cursor_type& cur, distance_t off) -> cursor_type&
    {
        return cur += narrow_cast<std::ptrdiff_t>(off);
    }

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr auto distance(Self&, cursor_type from, cursor_type to) -> distance_t
    {
        return narrow_cast<distance_t>(to) - narrow_cast<distance_t>(from);
    }

    template <decays_to<R> Self>
        requires std::ranges::contiguous_range<Self>
    static constexpr auto data(Self& self)
    {
        return std::ranges::data(self);
    }

    template <decays_to<R> Self, typename Pred>
        requires std::ranges::range<Self>
    static constexpr auto for_each_while(Self& self, Pred pred) -> cursor_type
    {
        cursor_type idx = 0;
        cursor_type const last = std::ranges::size(self);
        auto* data = std::ranges::data(self);

        for (; idx != last; ++idx) {
            if (!std::invoke(pred, data[idx])) {
                break;
            }
        }

        return idx;
    }
};

} // namespace flux

#endif // FLUX_RANGE_FROM_RANGE_HPP_INCLUDED
