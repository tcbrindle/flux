
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SLICE_HPP_INCLUDED
#define FLUX_OP_SLICE_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>

namespace flux {

namespace detail {

template <cursor Cur, bool Bounded>
struct slice_data {
    Cur first;
    Cur last;
};

template <cursor Cur>
struct slice_data<Cur, false> {
    Cur first;
};

template <sequence Base, bool Bounded>
    requires (!Bounded || regular_cursor<cursor_t<Base>>)
struct subsequence : lens_base<subsequence<Base, Bounded>>
{
private:
    Base* base_;
    FLUX_NO_UNIQUE_ADDRESS slice_data<cursor_t<Base>, Bounded> data_;

    friend struct sequence_iface<subsequence>;

public:
    constexpr subsequence(Base& base, cursor_t<Base>&& from,
                          cursor_t<Base>&& to)
        requires Bounded
        : base_(std::addressof(base)),
          data_{std::move(from), std::move(to)}
    {}

    constexpr subsequence(Base& base, cursor_t<Base>&& from)
        requires (!Bounded)
        : base_(std::addressof(base)),
          data_{std::move(from)}
    {}

    constexpr auto base() const -> Base& { return *base_; };
};

template <sequence Seq>
subsequence(Seq&, cursor_t<Seq>, cursor_t<Seq>) -> subsequence<Seq, true>;

template <sequence Seq>
subsequence(Seq&, cursor_t<Seq>) -> subsequence<Seq, false>;

template <typename Seq>
concept has_overloaded_slice =
    requires (Seq& seq, cursor_t<Seq> cur) {
        { iface_t<Seq>::slice(seq, std::move(cur)) } -> sequence;
        { iface_t<Seq>::slice(seq, std::move(cur), std::move(cur)) } -> sequence;
    };

struct slice_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq> from,
                              cursor_t<Seq> to) const
        -> sequence auto
    {
        if constexpr (has_overloaded_slice<Seq>) {
            return iface_t<Seq>::slice(seq, std::move(from), std::move(to));
        } else {
            return subsequence(seq, std::move(from), std::move(to));
        }
    }

    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq> from, last_fn) const
        -> sequence auto
    {
        if constexpr (has_overloaded_slice<Seq>) {
            return iface_t<Seq>::slice(seq, std::move(from));
        } else {
            return subsequence(seq, std::move(from));
        }
    }
};

} // namespace detail

using detail::subsequence;

template <typename Base, bool Bounded>
struct sequence_iface<subsequence<Base, Bounded>>
    : detail::passthrough_iface_base<Base>
{
    using self_t = subsequence<Base, Bounded>;

    static constexpr auto first(self_t& self) -> cursor_t<Base>
    {
        if constexpr (std::copy_constructible<decltype(self.data_.first)>) {
            return self.data_.first;
        } else {
            return std::move(self.data_.first);
        }
    }

    static constexpr bool is_last(self_t& self, cursor_t<Base> const& cur) {
        if constexpr (Bounded) {
            return cur == self.data_.last;
        } else {
            return flux::is_last(*self.base_, cur);
        }
    }

    static constexpr auto last(self_t& self) -> cursor_t<Base>
        requires (Bounded || bounded_sequence<Base>)
    {
        if constexpr (Bounded) {
            return self.data_.last;
        } else {
            return flux::last(*self.base_);
        }
    }

    static constexpr auto data(self_t& self)
        requires contiguous_sequence<Base>
    {
        return flux::data(*self.base_) +
               flux::distance(*self.base_, flux::first(*self.base_), self.data_.first);
    }

    void size() = delete;
    void for_each_while() = delete;
};

inline constexpr auto slice = detail::slice_fn{};

#if 0
template <typename Derived>
template <std::same_as<Derived> D>
constexpr auto lens_base<Derived>::slice(cursor_t<D> from, cursor_t<D> to) &
{
    return flux::slice(derived(), std::move(from), std::move(to));
}

template <typename Derived>
template <std::same_as<Derived> D>
constexpr auto lens_base<Derived>::slice_from(cursor_t<D> from) &
{
    return flux::slice(derived(), std::move(from));
}
#endif

} // namespace flux

#endif // namespace FLUX_OP_SLICE_HPP_INCLUDED
