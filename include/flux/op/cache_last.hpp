
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CACHE_LAST_HPP_INCLUDED
#define FLUX_OP_CACHE_LAST_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>

namespace flux {

namespace detail {

template <sequence Base>
struct cache_last_adaptor : inline_sequence_base<cache_last_adaptor<Base>>
{
private:
    Base base_;
    flux::optional<cursor_t<Base>> cached_last_{};

    friend struct passthrough_traits_base<Base>;

    constexpr auto base() -> Base& { return base_; }

public:
    constexpr explicit cache_last_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_sequence_traits : detail::passthrough_traits_base<Base> {

        using value_type = value_t<Base>;
        using self_t = cache_last_adaptor;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;

        static constexpr auto is_last(self_t& self, cursor_t<Base> const& cur)
        {
            if (flux::is_last(self.base_, cur)) {
                self.cached_last_ = flux::optional(cur);
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
                FLUX_DEBUG_ASSERT(self.cached_last_.has_value());
            }
            return self.cached_last_.value_unchecked();
        }
    };
};

struct cache_last_fn {
    template <adaptable_sequence Seq>
        requires bounded_sequence<Seq> ||
            (multipass_sequence<Seq> && not infinite_sequence<Seq>)
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr (bounded_sequence<Seq>) {
            return FLUX_FWD(seq);
        } else {
            return cache_last_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto cache_last = detail::cache_last_fn{};

template <typename Derived>
constexpr auto inline_sequence_base<Derived>::cache_last() &&
    requires bounded_sequence<Derived> ||
        (multipass_sequence<Derived> && not infinite_sequence<Derived>)
{
    return flux::cache_last(std::move(derived()));
}

} // namespace flux

#endif // FLUX_OP_CACHE_LAST_HPP_INCLUDED
