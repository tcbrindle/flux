
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED
#define FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED

#include <flux/core/sequence_access.hpp>

#include <initializer_list>
#include <functional>

namespace flux {

/*
 * Default implementation for C arrays of known bound
 */
template <typename T, index_t N>
struct sequence_traits<T[N]> {

    static constexpr auto first(auto const&) -> index_t { return index_t{0}; }

    static constexpr bool is_last(auto const&, index_t idx) { return idx >= N; }

    static constexpr auto read_at(auto& self, index_t idx) -> decltype(auto)
    {
        bounds_check(idx >= 0);
        bounds_check(idx < N);
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
 * Default implementation for std::initializer_list
 */
template <typename T>
struct sequence_traits<std::initializer_list<T>> {

    using ilist_t = std::initializer_list<T>;

    static constexpr auto first(ilist_t self) -> T const* { return self.begin(); }

    static constexpr auto is_last(ilist_t self, T const* ptr) -> bool
    {
        return ptr == self.end();
    }

    static constexpr auto read_at(ilist_t, T const* ptr) -> T const& { return *ptr; }

    static constexpr auto inc(ilist_t, T const*& ptr) -> T const*& { return ++ptr; }

    static constexpr auto last(ilist_t self) -> T const* { return self.end(); }

    static constexpr auto dec(ilist_t, T const*& ptr) -> T const*& { return --ptr; }

    static constexpr auto inc(ilist_t, T const*& ptr, std::ptrdiff_t offset) -> T const*&
    {
        return ptr += offset;
    }

    static constexpr auto distance(ilist_t, T const* from, T const* to) -> std::ptrdiff_t
    {
        return to - from;
    }

    static constexpr auto data(ilist_t self) -> T const* { return std::data(self); }

    static constexpr auto size(ilist_t self) { return self.size(); }
};

/*
 * Default implementation for std::reference_wrapper<T>
 */
template <sequence Seq>
struct sequence_traits<std::reference_wrapper<Seq>> {

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

} // namespace flux

#endif // FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED
