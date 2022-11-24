
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_DROP_HPP_INCLUDED
#define FLUX_OP_DROP_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>

#include <optional>

namespace flux {

namespace detail {

template <sequence Base>
struct drop_adaptor : lens_base<drop_adaptor<Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    distance_t count_;
    std::optional<cursor_t<Base>> cached_first_;

public:
    constexpr drop_adaptor(decays_to<Base> auto&& base, distance_t count)
        : base_(FLUX_FWD(base)),
          count_(count)
    {}

    constexpr Base& base() & { return base_; }
    constexpr Base const& base() const& { return base_; }

    struct flux_sequence_iface : passthrough_iface_base<drop_adaptor> {
        using value_type = value_t<Base>;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;

        static constexpr auto first(drop_adaptor& self)
        {
            if constexpr (std::copy_constructible<cursor_t<Base>>) {
                if (!self.cached_first_) {
                    self.cached_first_ = flux::next(self.base_, flux::first(self.base()), self.count_);
                }

                return *self.cached_first_;
            } else {
                return flux::next(self.base_, flux::first(self.base()), self.count_);
            }
        }

        static constexpr auto size(drop_adaptor& self)
            requires sized_sequence<Base>
        {
            return flux::size(self.base()) - self.count_;
        }

        static constexpr auto data(drop_adaptor& self)
            requires contiguous_sequence<Base>
        {
            return flux::data(self.base()) + self.count_;
        }

        void for_each_while(...) = delete;
    };
};

struct drop_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, distance_t count) const
    {
        return drop_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq), count);
    }

};

} // namespace detail

inline constexpr auto drop = detail::drop_fn{};

template <typename Derived>
constexpr auto lens_base<Derived>::drop(distance_t count) &&
{
    return detail::drop_adaptor<Derived>(std::move(derived()), count);
}

} // namespace flux

#endif
