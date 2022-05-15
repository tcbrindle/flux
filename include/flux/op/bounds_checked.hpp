
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_BOUNDS_CHECKED_HPP_INCLUDED
#define FLUX_OP_BOUNDS_CHECKED_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>
#include <flux/op/slice.hpp>

namespace flux {

namespace detail {

template <lens Base>
struct bounds_checked_adaptor : lens_base<bounds_checked_adaptor<Base>>
{
private:
    Base base_;

    friend struct sequence_iface<bounds_checked_adaptor>;

public:
    constexpr explicit bounds_checked_adaptor(Base&& base)
        : base_(std::move(base))
    {}

    constexpr auto base() -> Base& { return base_; }
    constexpr auto base() const -> Base const& { return base_; }
};

struct bounds_checked_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        return bounds_checked_adaptor(flux::from(FLUX_FWD(seq)));
    }
};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::bounds_checked_adaptor<Base>>
    : detail::passthrough_iface_base<Base>
{
    static constexpr auto read_at(auto& self, auto const& cur)
        -> decltype(flux::checked_read_at(self.base_, cur))
    {
        return flux::checked_read_at(self.base_, cur);
    }

    static constexpr auto inc(auto& self, auto& cur)
        -> decltype(flux::checked_inc(self.base_, cur))
    {
        return flux::checked_inc(self.base_, cur);
    }

    static constexpr auto dec(auto& self, auto& cur)
        -> decltype(flux::checked_dec(self.base_, cur))
    {
        return flux::checked_dec(self.base_, cur);
    }

    static constexpr auto inc(auto& self, auto& cur, auto offset)
        -> decltype(flux::checked_inc(self.base_, cur, offset))
    {
        return flux::checked_inc(self.base_, cur, offset);
    }

    static constexpr auto move_at(auto& self, auto const& cur)
        -> decltype(flux::checked_move_at(self.base_, cur))
    {
        return flux::checked_move_at(self.base_, cur);
    }

    static constexpr auto slice(auto& self, auto first, auto last)
    {
        return detail::bounds_checked_adaptor(
            flux::from(flux::slice(self.base_, std::move(first), std::move(last))));
    }

    static constexpr auto slice(auto& self, auto first)
    {
        return detail::bounds_checked_adaptor(flux::from(flux::slice(self.base_, std::move(first), flux::last)));
    }
};

inline constexpr auto bounds_checked = detail::bounds_checked_fn{};

} // namespace flux

#endif // FLUX_OP_BOUNDS_CHECKED_HPP_INCLUDED
