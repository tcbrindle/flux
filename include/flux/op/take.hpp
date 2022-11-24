
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_TAKE_HPP_INCLUDED
#define FLUX_OP_TAKE_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/for_each_while.hpp>
#include <flux/op/slice.hpp>

namespace flux {

namespace detail {

template <sequence Base>
struct take_adaptor : lens_base<take_adaptor<Base>>
{
private:
    Base base_;
    distance_t count_;

    template <bool IsConst>
    struct cursor_type {
    private:
        using base_t = std::conditional_t<IsConst, Base const, Base>;

    public:
        cursor_t<base_t> base_cur;
        distance_t length;

        friend bool operator==(cursor_type const&, cursor_type const&) = default;
        friend auto operator<=>(cursor_type const& lhs, cursor_type const& rhs) = default;
    };

    friend struct sequence_iface<take_adaptor>;

public:
    constexpr take_adaptor(decays_to<Base> auto&& base, distance_t count)
        : base_(FLUX_FWD(base)),
          count_(count)
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base&& { return std::move(base_); }
};

struct take_fn {

    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, distance_t count) const
    {
        if constexpr (random_access_sequence<Seq> && std::is_lvalue_reference_v<Seq>) {
            auto first = flux::first(seq);
            auto last = flux::next(seq, first, count);
            return flux::from(flux::slice(seq, std::move(first), std::move(last)));
        } else {
            return take_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq), count);
        }
    }
};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::take_adaptor<Base>> {

    template <typename Self>
    using cursor_t =
        typename std::remove_const_t<Self>::template cursor_type<std::is_const_v<Self>>;

    using value_type = value_t<Base>;

    static constexpr bool disable_multipass = !multipass_sequence<Base>;

    template <typename Self>
    static constexpr auto first(Self& self)
    {
        return cursor_t<Self>{
            .base_cur = flux::first(self.base_),
            .length = self.count_
        };
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, cursor_t<Self> const& cur) -> bool
    {
        return cur.length <= 0 || flux::is_last(self.base_, cur.base_cur);
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
    {
        return flux::read_at(self.base_, cur.base_cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur) -> cursor_t<Self>&
    {
        flux::inc(self.base_, cur.base_cur);
        --cur.length;
        return cur;
    }

    template <typename Self>
    static constexpr auto dec(Self& self, cursor_t<Self>& cur) -> cursor_t<Self>&
        requires bidirectional_sequence<Base>
    {
        flux::dec(self.base_, cur.base_cur);
        ++cur.length;
        return cur;
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur, distance_t offset)
        -> cursor_t<Self>&
        requires random_access_sequence<Base>
    {
        flux::inc(self.base_, cur.base_cur, offset);
        cur.length -= offset;
        return cur;
    }

    template <typename Self>
    static constexpr auto distance(Self& self, cursor_t<Self> const& from, cursor_t<Self> const& to)
        requires random_access_sequence<Base>
    {
        return std::min(flux::distance(self.base_, from.base_cur, to.base_cur),
                        from.length - to.length);
    }

    template <typename Self>
    static constexpr auto data(Self& self)
    {
        return flux::data(self.base_);
    }

    template <typename Self>
    static constexpr auto last(Self& self)
        requires random_access_sequence<Base> && sized_sequence<Base>
    {
        return cursor_t<Self>{
            .base_cur = flux::next(self.base_, flux::first(self.base_), size(self)),
            .length = 0
        };
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        requires sized_sequence<Base>
    {
        return std::min(flux::size(self.base_), self.count_);
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
    {
        return flux::move_at(self.base_, cur.base_cur);
    }
};

inline constexpr auto take = detail::take_fn{};

template <typename Derived>
constexpr auto lens_base<Derived>::take(distance_t count) &&
{
    return detail::take_adaptor<Derived>(std::move(derived()), count);
}

} // namespace flux

#endif // FLUX_OP_TAKE_HPP_INCLUDED
