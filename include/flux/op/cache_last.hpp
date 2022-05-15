
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CACHE_LAST_HPP_INCLUDED
#define FLUX_OP_CACHE_LAST_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>

#include <optional>

namespace flux {

namespace detail {

template <lens Base>
struct cache_last_adaptor : lens_base<cache_last_adaptor<Base>>
{
private:
    Base base_;
    std::optional<cursor_t<Base>> cached_last_{};

    friend struct sequence_iface<cache_last_adaptor>;
    friend struct passthrough_iface_base<Base>;

    constexpr auto base() -> Base& { return base_; }

public:
    constexpr explicit cache_last_adaptor(Base&& base)
        : base_(std::move(base))
    {}
};

struct cache_last_fn {
    template <adaptable_sequence Seq>
        requires bounded_sequence<Seq> ||
            (multipass_sequence<Seq> && not infinite_sequence<Seq>)
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr (bounded_sequence<Seq>) {
            return flux::from(FLUX_FWD(seq));
        } else {
            return cache_last_adaptor(flux::from(FLUX_FWD(seq)));
        }
    }
};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::cache_last_adaptor<Base>>
    : detail::passthrough_iface_base<Base> {

    using self_t = detail::cache_last_adaptor<Base>;

    static constexpr auto is_last(self_t& self, cursor_t<self_t> const& cur)
    {
        if (flux::is_last(self.base_, cur)) {
            self.cached_last_ = cur;
            return true;
        } else {
            return false;
        }
    }

    static constexpr auto last(self_t& self)
    {
        if (!self.cached_last_) {
            auto cur = flux::first(self);
            while (!is_last(self, cur)) {
                flux::inc(self.base_, cur);
            }
            assert(self.cached_last_.has_value());
        }
        return *self.cached_last_;
    }
};

inline constexpr auto cache_last = detail::cache_last_fn{};

template <typename Derived>
constexpr auto lens_base<Derived>::cache_last() &&
    requires bounded_sequence<Derived> ||
        (multipass_sequence<Derived> && not infinite_sequence<Derived>)
{
    return flux::cache_last(std::move(derived()));
}

} // namespace flux

#endif // FLUX_OP_CACHE_LAST_HPP_INCLUDED
