
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_SIMPLE_SEQUENCE_BASE_HPP_INCLUDED
#define FLUX_CORE_SIMPLE_SEQUENCE_BASE_HPP_INCLUDED

#include <flux/core/lens_base.hpp>

namespace flux {

template <typename D>
struct simple_sequence_base : lens_base<D> {};

namespace detail {

template <typename O>
concept optional_like =
    std::default_initializable<O> &&
    std::movable<O> &&
    requires (O& o) {
        { static_cast<bool>(o) };
        { *o } -> flux::detail::can_reference;
    };

template <typename S>
concept simple_sequence =
    std::derived_from<S, simple_sequence_base<S>> &&
    requires (S& s) {
        { s.maybe_next() } -> optional_like;
    };

} // namespace detail

template <detail::simple_sequence S>
struct sequence_traits<S> {
private:
    class cursor_type {
        friend struct sequence_traits;
        using optional_t = decltype(FLUX_DECLVAL(S&).maybe_next());
        optional_t opt_{};

        constexpr cursor_type() = default;

        constexpr explicit cursor_type(optional_t&& opt)
            : opt_(std::move(opt))
        {}

    public:
        cursor_type(cursor_type&&) = default;
        cursor_type& operator=(cursor_type&&) = default;
    };

public:
    static constexpr bool is_infinite = detail::is_infinite_seq<S>;

    static constexpr auto first(S& self) -> cursor_type
    {
        return cursor_type(self.maybe_next());
    }

    static constexpr auto is_last(S&, cursor_type const& cur) -> bool
    {
        return !static_cast<bool>(cur.opt_);
    }

    static constexpr auto inc(S& self, cursor_type& cur) -> cursor_type&
    {
        cur.opt_ = self.maybe_next();
        return cur;
    }

    static constexpr auto read_at(S&, cursor_type const& cur) -> decltype(auto)
    {
        return *cur.opt_;
    }

    static constexpr auto for_each_while(S& self, auto&& pred) -> cursor_type
    {
        while (auto o = self.maybe_next()) {
            if (!std::invoke(pred, *o)) {
                return cursor_type(std::move(o));
            }
        }
        return cursor_type{};
    }
};

} // namespace flux

#endif // FLUX_CORE_SIMPLE_SEQUENCE_BASE_HPP_INCLUDED


