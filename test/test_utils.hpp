
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
    static constexpr bool impl(flux::iterable auto&& it1, flux::iterable auto&& it2)
    {
        using namespace flux;

        auto ctx1 = iterate(it1);
        auto ctx2 = iterate(it2);

        while (true) {
            auto opt1 = next_element(ctx1);
            auto opt2 = next_element(ctx2);

            if (opt1.has_value() && opt2.has_value()) {
                if (*opt1 != *opt2) {
                    return false;
                }
            } else if (opt1.has_value() || opt2.has_value()) {
                return false;
            } else {
                return true;
            }
        }
    }

public:
    template <typename T>
    constexpr bool operator()(flux::iterable auto&& seq, std::initializer_list<T> ilist) const
    {
        return impl(FLUX_FWD(seq), ilist);
    }

    constexpr bool operator()(flux::iterable auto&& seq1, flux::iterable auto&& seq2) const
    {
        return impl(FLUX_FWD(seq1), FLUX_FWD(seq2));
    }

} check_equal;

template <flux::sequence Base>
struct single_pass_only : flux::inline_sequence_base<single_pass_only<Base>> {
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

    friend struct flux::sequence_traits<single_pass_only>;

public:
    constexpr explicit single_pass_only(Base&& base)
        : base_(std::move(base)) {}

    // Move-only sequence is useful for testing
    single_pass_only(single_pass_only&&) = default;
    single_pass_only&  operator=(single_pass_only&&) = default;
};

template <flux::iterable Base>
struct iterable_only {
private:
    Base base_;

public:
    constexpr explicit iterable_only(flux::decays_to<Base> auto&& base) : base_(FLUX_FWD(base)) { }

    constexpr auto iterate() { return flux::iterate(base_); }

    constexpr auto iterate() const
        requires flux::iterable<Base const>
    {
        return flux::iterate(base_);
    }

    constexpr auto reverse_iterate()
        requires flux::reverse_iterable<Base>
    {
        return flux::reverse_iterate(base_);
    }

    constexpr auto reverse_iterate() const
        requires flux::reverse_iterable<Base const>
    {
        return flux::reverse_iterate(base_);
    }

    constexpr auto size() -> flux::int_t
        requires flux::sized_iterable<Base>
    {
        return flux::iterable_size(base_);
    }

    constexpr auto size() const -> flux::int_t
        requires flux::sized_iterable<Base const>
    {
        return flux::iterable_size(base_);
    }
};

template <flux::iterable Base>
iterable_only(Base) -> iterable_only<Base>;
}

template <typename Base>
struct flux::sequence_traits<single_pass_only<Base>> : flux::default_sequence_traits
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
        requires sized_sequence<Base>
    {
        return flux::size(self.base_);
    }
};
