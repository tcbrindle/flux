
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <doctest/doctest.h>

#include <stdexcept>

#ifndef USE_MODULES
#include <flux.hpp>
#else
#include <flux/macros.hpp>
import flux;
#endif

#define STATIC_CHECK(...) if (!(__VA_ARGS__)) throw std::runtime_error("Test assertion failed")

inline namespace test_utils {

inline constexpr struct {
private:
    static constexpr bool impl(flux::sequence auto&& seq1, flux::sequence auto&& seq2)
    {
        using namespace flux;

        auto cur1 = first(seq1);
        auto cur2 = first(seq2);

        while (!is_last(seq1, cur1) && !is_last(seq2, cur2)) {
            if (read_at(seq1, cur1) != read_at(seq2, cur2)) { return false; }

            inc(seq1, cur1);
            inc(seq2, cur2);
        }

        return is_last(seq1, cur1) == is_last(seq2, cur2);
    }

public:
    template <typename T>
    constexpr bool operator()(flux::iterable auto&& it,
                              std::initializer_list<T> ilist) const
    {
        auto iter = ilist.begin();
        bool r =  flux::iterate(it, [&iter](auto&& elem) {
            return *iter++ == elem;
        });
        return r ? iter == ilist.end() : r;
    }

    template <typename T>
    constexpr bool operator()(flux::sequence auto&& seq,
                              std::initializer_list<T> ilist) const
    {
        return impl(FLUX_FWD(seq), ilist);
    }

    constexpr bool operator()(flux::iterable auto&& it,
                              flux::sequence auto&& seq) const
    {
        auto cur = flux::first(seq);
        bool k = flux::iterate(it, [&](auto&& elem) {
            bool r = (elem == flux::read_at(seq, cur));
            flux::inc(seq, cur);
            return r;
        });
        return k ? flux::is_last(seq, cur) : k;
    }

    constexpr bool operator()(flux::sequence auto&& seq1,
                              flux::sequence auto&& seq2) const
    {
        return impl(FLUX_FWD(seq1), FLUX_FWD(seq2));
    }

} check_equal;

template <flux::sequence Base>
struct single_pass_only : flux::inline_iter_base<single_pass_only<Base>> {
private:
    Base base_;

    struct cursor_type {
        flux::cursor_t<Base> base_cur;

        constexpr explicit(false) cursor_type(flux::cursor_t<Base>&& base_cur)
            : base_cur(std::move(base_cur))
        {
        }

        cursor_type(cursor_type&&) = default;
        cursor_type& operator=(cursor_type&&) = default;
    };

    friend struct flux::iter_traits<single_pass_only>;

public:
    constexpr explicit single_pass_only(Base&& base)
        : base_(std::move(base)) {}

    // Move-only sequence is useful for testing
    single_pass_only(single_pass_only&&) = default;
    single_pass_only&  operator=(single_pass_only&&) = default;
};

}

template <typename Base>
struct flux::iter_traits<single_pass_only<Base>> : flux::default_iter_traits
{
    using self_t = single_pass_only<Base>;
    using cursor_t = typename single_pass_only<Base>::cursor_type;

    static constexpr bool disable_multipass = true;

    static constexpr auto first(self_t& self)
    {
        return cursor_t(flux::first(self.base_));
    }

    static constexpr auto is_last(self_t& self, cursor_t const& cur)
    {
        return flux::is_last(self.base_, cur.base_cur);
    }

    static constexpr void inc(self_t& self, cursor_t& cur)
    {
        flux::inc(self.base_, cur.base_cur);
    }

    static constexpr decltype(auto) read_at(self_t& self, cursor_t const& cur)
    {
        return flux::read_at(self.base_, cur.base_cur);
    }

    static constexpr auto last(self_t& self)
        requires bounded_sequence<Base>
    {
        return cursor_t(flux::last(self.base_));
    }

    static constexpr auto size(self_t& self)
        requires sized_iterable<Base>
    {
        return flux::size(self.base_);
    }
};

template <typename Reqd, typename Expr>
constexpr void assert_has_type(Expr&&)
{
    static_assert(std::same_as<Reqd, Expr>);
}
