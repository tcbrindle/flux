
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED
#define FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED

#include <flux/core/numeric.hpp>
#include <flux/core/sequence_access.hpp>

#include <functional>
#include <ranges>

namespace flux {

/*
 * Default implementation for C arrays of known bound
 */
template <typename T, index_t N>
struct sequence_traits<T[N]> : default_sequence_traits {

    static constexpr auto first(auto const&) -> index_t { return index_t{0}; }

    static constexpr bool is_last(auto const&, index_t idx) { return idx >= N; }

    static constexpr auto read_at(auto& self, index_t idx) -> decltype(auto)
    {
        indexed_bounds_check(idx, N);
        return self[idx];
    }

    static constexpr auto read_at_unchecked(auto& self, index_t idx) -> decltype(auto)
    {
        return self[idx];
    }

    static constexpr auto inc(auto const&, index_t& idx)
    {
        FLUX_DEBUG_ASSERT(idx < N);
        idx = num::add(idx, distance_t{1});
    }

    static constexpr auto last(auto const&) -> index_t { return N; }

    static constexpr auto dec(auto const&, index_t& idx)
    {
        FLUX_DEBUG_ASSERT(idx > 0);
        idx = num::sub(idx, distance_t{1});
    }

    static constexpr auto inc(auto const&, index_t& idx, distance_t offset)
    {
        FLUX_DEBUG_ASSERT(num::add(idx, offset) <= N);
        FLUX_DEBUG_ASSERT(num::add(idx, offset) >= 0);
        idx = num::add(idx, offset);
    }

    static constexpr auto distance(auto const&, index_t from, index_t to) -> distance_t
    {
        return num::sub(to, from);
    }

    static constexpr auto data(auto& self) -> auto* { return self; }

    static constexpr auto size(auto const&) -> distance_t { return N; }

    static constexpr auto for_each_while(auto& self, auto&& pred) -> index_t
    {
        index_t idx = 0;
        while (idx < N) {
            if (!std::invoke(pred, self[idx])) {
                break;
            }
            ++idx;
        }
        return idx;
    }
};

/*
 * Default implementation for std::reference_wrapper<T>
 */
template <iterable Seq>
struct sequence_traits<std::reference_wrapper<Seq>> : default_sequence_traits {

    using self_t = std::reference_wrapper<Seq>;

    using value_type = value_t<Seq>;

    static constexpr bool disable_multipass = !multipass_sequence<Seq>;

    static consteval auto element_type(self_t) -> element_t<Seq>;

    static constexpr auto iterate(self_t self, auto&& pred) -> bool
    {
        return flux::iterate(self.get(), FLUX_FWD(pred));
    }

    static constexpr auto first(self_t self) -> cursor_t<Seq>
    {
         return flux::first(self.get());
    }

    static constexpr bool is_last(self_t self, cursor_t<Seq> const& cur)
    {
        return flux::is_last(self.get(), cur);
    }

    static constexpr auto read_at(self_t self, cursor_t<Seq> const& cur)
        -> decltype(auto)
    {
        return flux::read_at(self.get(), cur);
    }

    static constexpr auto read_at_unchecked(self_t self, cursor_t<Seq> const& cur)
        -> decltype(auto)
    {
        return flux::read_at_unchecked(self.get(), cur);
    }

    static constexpr auto inc(self_t self, cursor_t<Seq>& cur)
        -> cursor_t<Seq>&
    {
        return flux::inc(self.get(), cur);
    }

    static constexpr auto dec(self_t self, cursor_t<Seq>& cur)
        -> cursor_t<Seq>&
        requires bidirectional_sequence<Seq>
    {
        return flux::dec(self.get(), cur);
    }

    static constexpr auto inc(self_t self, cursor_t<Seq>& cur, distance_t offset)
        -> cursor_t<Seq>&
        requires random_access_sequence<Seq>
    {
        return flux::inc(self.get(), cur,  offset);
    }

    static constexpr auto distance(self_t self, cursor_t<Seq> const& from,
                                   cursor_t<Seq> const& to)
        -> distance_t
        requires random_access_sequence<Seq>
    {
        return flux::distance(self.get(), from, to);
    }

    static constexpr auto data(self_t self)
        requires contiguous_sequence<Seq>
    {
        return flux::data(self.get());
    }

    static constexpr auto last(self_t self) -> cursor_t<Seq>
        requires bounded_sequence<Seq>
    {
        return flux::last(self.get());
    }

    static constexpr auto size(self_t self) -> distance_t
        requires sized_iterable<Seq>
    {
        return flux::size(self.get());
    }

    static constexpr auto move_at(self_t self, cursor_t<Seq> const& cur)
        -> rvalue_element_t<Seq>
    {
        return flux::move_at(self.get(), cur);
    }
};

namespace detail {

template <typename R>
concept contiguous_sized_range = std::ranges::contiguous_range<R> && std::ranges::sized_range<R>;

}

// Default implementation for ranges
template <typename R>
    requires (!detail::derived_from_inline_sequence_base<R> &&
               std::ranges::input_range<R>)
struct sequence_traits<R> : default_sequence_traits {

    using value_type = std::ranges::range_value_t<R>;

    template <std::ranges::input_range Self>
    static consteval auto element_type(Self&) -> std::ranges::range_reference_t<Self>;

    template <std::ranges::input_range Self, typename Pred>
    static constexpr auto iterate(Self& self, Pred&& pred) -> bool
    {
        for (auto&& elem : self) {
            if (!std::invoke(pred, FLUX_FWD(elem))) {
                return false;
            }
        }
        return true;
    }

    template <std::ranges::sized_range Self>
    static constexpr auto size(Self& self) -> distance_t
    {
        return num::cast<distance_t>(std::ranges::ssize(self));
    }

    template <detail::contiguous_sized_range Self>
    static constexpr auto first(Self&) -> index_t { return index_t{0}; }

    template <detail::contiguous_sized_range Self>
    static constexpr auto is_last(Self& self, index_t idx) -> bool
    {
        return idx >= size(self);
    }

    template <detail::contiguous_sized_range Self>
    static constexpr auto inc(Self& self, index_t& idx) -> void
    {
        FLUX_DEBUG_ASSERT(idx < size(self));
        idx = num::add(idx, distance_t{1});
    }

    template <detail::contiguous_sized_range Self>
    static constexpr auto read_at(Self& self, index_t idx)
        -> std::ranges::range_reference_t<Self>
    {
        indexed_bounds_check(idx, size(self));
        return data(self)[idx];
    }

    template <detail::contiguous_sized_range Self>
    static constexpr auto read_at_unchecked(Self& self, index_t idx)
        -> std::ranges::range_reference_t<Self>
    {
        return data(self)[idx];
    }

    template <detail::contiguous_sized_range Self>
    static constexpr auto dec(Self&, index_t& idx) -> void
    {
        FLUX_DEBUG_ASSERT(idx > 0);
        idx = num::sub(idx, distance_t{1});
    }

    template <detail::contiguous_sized_range Self>
    static constexpr auto last(Self& self) -> index_t { return size(self); }

    template <detail::contiguous_sized_range Self>
    static constexpr auto inc(Self& self, index_t& idx, distance_t offset) -> void
    {
        FLUX_DEBUG_ASSERT(num::add(idx, offset) <= size(self));
        FLUX_DEBUG_ASSERT(num::add(idx, offset) >= 0);
        idx = num::add(idx, offset);
    }

    template <detail::contiguous_sized_range Self>
    static constexpr auto distance(Self&, index_t from, index_t to) -> distance_t
    {
        return num::sub(to, from);
    }

    template <detail::contiguous_sized_range Self>
    static constexpr auto data(Self& self) -> auto*
    {
        return std::ranges::data(self);
    }

    template <detail::contiguous_sized_range Self>
    static constexpr auto for_each_while(Self& self, auto&& pred) -> index_t
    {
        auto iter = std::ranges::begin(self);
        auto const end = std::ranges::end(self);

        while (iter != end) {
            if (!std::invoke(pred, *iter)) {
                break;
            }
            ++iter;
        }

        return num::cast<index_t>(iter - std::ranges::begin(self));
    }
};

} // namespace flux

#endif // FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED
