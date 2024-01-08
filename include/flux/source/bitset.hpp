
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_BITSET_HPP_INCLUDED
#define FLUX_SOURCE_BITSET_HPP_INCLUDED

#include <flux/core.hpp>

#include <bitset>

namespace flux {

template <std::size_t N>
struct sequence_traits<std::bitset<N>> {

    using value_type = bool;

    using self_t = std::bitset<N>;

    static constexpr auto first(self_t const&) -> std::size_t { return 0; }

    static constexpr auto is_last(self_t const&, std::size_t idx) { return idx == N; }

    static constexpr auto read_at(self_t& self, std::size_t idx)
        -> typename std::bitset<N>::reference
    {
        return self[idx];
    }

    static constexpr auto read_at(self_t const& self, std::size_t idx) -> bool
    {
        return self[idx];
    }

    static constexpr auto move_at(self_t const& self, std::size_t idx) -> bool
    {
        return self[idx];
    }

    static constexpr auto inc(self_t const&, std::size_t& idx) -> std::size_t&
    {
        return ++idx;
    }

    static constexpr auto dec(self_t const&, std::size_t& idx) -> std::size_t&
    {
        return --idx;
    }

    static constexpr auto inc(self_t const&, std::size_t& idx, std::ptrdiff_t off)
        -> std::size_t&
    {
        return idx += static_cast<std::size_t>(off);
    }

    static constexpr auto distance(self_t const&, std::size_t from, std::size_t to)
        -> std::ptrdiff_t
    {
        return static_cast<std::ptrdiff_t>(to) - static_cast<std::ptrdiff_t>(from);
    }

    static constexpr auto last(self_t const&) -> std::size_t { return N; }

    static constexpr auto size(self_t const&) -> std::ptrdiff_t { return N; }

};


} // namespace flux

#endif
