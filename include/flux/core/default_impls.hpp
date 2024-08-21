
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
        idx = num::checked_add(idx, distance_t{1});
    }

    static constexpr auto last(auto const&) -> index_t { return N; }

    static constexpr auto dec(auto const&, index_t& idx)
    {
        FLUX_DEBUG_ASSERT(idx > 0);
        idx = num::checked_sub(idx, distance_t{1});
    }

    static constexpr auto inc(auto const&, index_t& idx, distance_t offset)
    {
        FLUX_DEBUG_ASSERT(num::checked_add(idx, offset) <= N);
        FLUX_DEBUG_ASSERT(num::checked_add(idx, offset) >= 0);
        idx = num::checked_add(idx, offset);
    }

    static constexpr auto distance(auto const&, index_t from, index_t to) -> distance_t
    {
        return num::checked_sub(to, from);
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
template <sequence Seq>
struct sequence_traits<std::reference_wrapper<Seq>> : default_sequence_traits {

    using self_t = std::reference_wrapper<Seq>;

    using value_type = value_t<Seq>;

    static constexpr bool disable_multipass = !multipass_sequence<Seq>;

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
        requires sized_sequence<Seq>
    {
        return flux::size(self.get());
    }

    static constexpr auto move_at(self_t self, cursor_t<Seq> const& cur)
        -> rvalue_element_t<Seq>
    {
        return flux::move_at(self.get(), cur);
    }
};

// Default implementation for contiguous, sized ranges
template <typename R>
    requires (!detail::derived_from_inline_sequence_base<R> &&
             std::ranges::contiguous_range<R> &&
             std::ranges::sized_range<R> &&
             std::ranges::contiguous_range<R const> &&
             std::ranges::sized_range<R const>)
struct sequence_traits<R> : default_sequence_traits {

    using value_type = std::ranges::range_value_t<R>;

    static constexpr auto first(auto&) -> index_t { return index_t{0}; }

    static constexpr auto is_last(auto& self, index_t idx)
    {
        return idx >= size(self);
    }

    static constexpr auto inc(auto& self, index_t& idx)
    {
        FLUX_DEBUG_ASSERT(idx < size(self));
        idx = num::checked_add(idx, distance_t{1});
    }

    static constexpr auto read_at(auto& self, index_t idx) -> decltype(auto)
    {
        indexed_bounds_check(idx, size(self));
        return data(self)[idx];
    }

    static constexpr auto read_at_unchecked(auto& self, index_t idx) -> decltype(auto)
    {
        return data(self)[idx];
    }

    static constexpr auto dec(auto&, index_t& idx)
    {
        FLUX_DEBUG_ASSERT(idx > 0);
        idx = num::checked_sub(idx, distance_t{1});
    }

    static constexpr auto last(auto& self) -> index_t { return size(self); }

    static constexpr auto inc(auto& self, index_t& idx, distance_t offset)
    {
        FLUX_DEBUG_ASSERT(num::checked_add(idx, offset) <= size(self));
        FLUX_DEBUG_ASSERT(num::checked_add(idx, offset) >= 0);
        idx = num::checked_add(idx, offset);
    }

    static constexpr auto distance(auto&, index_t from, index_t to) -> distance_t
    {
        return num::checked_sub(to, from);
    }

    static constexpr auto size(auto& self) -> distance_t
    {
        return num::checked_cast<distance_t>(std::ranges::ssize(self));
    }

    static constexpr auto data(auto& self) -> auto*
    {
        return std::ranges::data(self);
    }

    static constexpr auto for_each_while(auto& self, auto&& pred) -> index_t
    {
        auto iter = std::ranges::begin(self);
        auto const end = std::ranges::end(self);

        while (iter != end) {
            if (!std::invoke(pred, *iter)) {
                break;
            }
            ++iter;
        }

        return num::checked_cast<index_t>(iter - std::ranges::begin(self));
    }
};

} // namespace flux

#endif // FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED
