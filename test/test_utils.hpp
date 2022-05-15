
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <flux/core.hpp>
#include <flux/op/from.hpp>
#include <flux/op/ref.hpp>

#define STATIC_CHECK(...) if (!(__VA_ARGS__)) return false

inline namespace test_utils {

inline constexpr struct {
private:
    static constexpr bool impl(flux::lens auto seq1, flux::lens auto seq2)
    {
        auto cur1 = seq1.first();
        auto cur2 = seq2.first();

        while (!seq1.is_last(cur1) && !seq2.is_last(cur2)) {
            if (seq1[cur1] != seq2[cur2]) { return false; }

            seq1.inc(cur1);
            seq2.inc(cur2);
        }

        return seq1.is_last(cur1) == seq2.is_last(cur2);
    }

public:
    template <typename T>
    constexpr bool operator()(flux::sequence auto&& seq,
                              std::initializer_list<T> ilist) const
    {
        return impl(flux::from(FLUX_FWD(seq)), flux::from(ilist));
    }

    constexpr bool operator()(flux::sequence auto&& seq1,
                              flux::sequence auto&& seq2) const
    {
        return impl(flux::from(FLUX_FWD(seq1)), flux::from(FLUX_FWD(seq2)));
    }

} check_equal;

template <flux::lens Base>
struct single_pass_only : flux::lens_base<single_pass_only<Base>> {
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

    friend struct flux::sequence_iface<single_pass_only>;

public:
    constexpr explicit single_pass_only(Base&& base)
        : base_(std::move(base)) {}

    // Move-only sequence is useful for testing
    single_pass_only(single_pass_only&&) = default;
    single_pass_only&  operator=(single_pass_only&&) = default;
};

}

template <typename Base>
struct flux::sequence_iface<single_pass_only<Base>>
{
    using self_t = single_pass_only<Base>;
    using cursor_t = typename single_pass_only<Base>::cursor_type;

    static constexpr bool disable_multipass = true;

    static constexpr auto first(self_t& self)
    {
        return cursor_t(self.base_.first());
    }

    static constexpr auto is_last(self_t& self, cursor_t const& cur)
    {
        return self.base_.is_last(cur.base_cur);
    }

    static constexpr auto& inc(self_t& self, cursor_t& cur)
    {
        self.base_.inc(cur.base_cur);
        return cur;
    }

    static constexpr decltype(auto) read_at(self_t& self, cursor_t const& cur)
    {
        return self.base_.read_at(cur.base_cur);
    }

    static constexpr auto last(self_t& self)
        requires bounded_sequence<Base>
    {
        return cursor_t(self.base_.last());
    }

    static constexpr auto size(self_t& self)
        requires sized_sequence<Base>
    {
        return self.base_.size();
    }
};

template <typename Reqd, typename Expr>
constexpr void assert_has_type(Expr&&)
{
    static_assert(std::same_as<Reqd, Expr>);
}
