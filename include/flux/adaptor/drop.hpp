
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_DROP_HPP_INCLUDED
#define FLUX_ADAPTOR_DROP_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/adaptor/stride.hpp>

namespace flux {

namespace detail {

template <iterable Base>
struct drop_adaptor : inline_sequence_base<drop_adaptor<Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    distance_t count_;

public:
    constexpr drop_adaptor(decays_to<Base> auto&& base, distance_t count)
        : base_(FLUX_FWD(base)),
          count_(count)
    {}

    constexpr Base& base() & { return base_; }
    constexpr Base const& base() const& { return base_; }

    struct flux_sequence_traits : passthrough_traits_base {
        using value_type = value_t<Base>;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;

        static constexpr auto iterate(auto& self, auto&& pred) -> bool
        {
            if constexpr (random_access_sequence<Base> && bounded_sequence<Base>) {
                auto cur = flux::first(self.base_);
                flux::inc(self.base_, cur, self.count_);
                return flux::iterate(flux::slice(self.base_, cur, flux::last(self.base_)), FLUX_FWD(pred));
            } else {
                distance_t n = 0;
                return flux::iterate(self.base_, [&pred, &n, count = self.count_](auto&& elem) {
                    if (n < count) {
                        ++n;
                        return true; // continue
                    } else {
                        return std::invoke(pred, FLUX_FWD(elem));
                    }
                });
            }
        }

        static constexpr auto first(auto& self) -> decltype(flux::first(self.base()))
            requires sequence<decltype(self.base())>
        {
            auto cur = flux::first(self.base_);
            detail::advance(self.base_, cur, self.count_);
            return cur;
        }

        static constexpr auto size(auto& self) -> distance_t
            requires sized_iterable<Base>
        {
            return (cmp::max)(num::sub(flux::size(self.base()), self.count_),
                              distance_t{0});
        }

        static constexpr auto data(auto& self)
            requires contiguous_sequence<Base> && sized_iterable<Base>
        {
            return flux::data(self.base()) + (cmp::min)(self.count_, flux::size(self.base_));
        }
    };
};

struct drop_fn {
    template <sink_iterable It>
    [[nodiscard]]
    constexpr auto operator()(It&& it, num::integral auto count) const
    {
        auto count_ = num::cast<distance_t>(count);
        if (count_ < 0) {
            runtime_error("Negative argument passed to drop()");
        }

        return drop_adaptor<std::decay_t<It>>(FLUX_FWD(it), count_);
    }

};

} // namespace detail

FLUX_EXPORT inline constexpr auto drop = detail::drop_fn{};

template <typename Derived>
constexpr auto inline_sequence_base<Derived>::drop(num::integral auto count) &&
{
    return flux::drop(std::move(derived()), count);
}

} // namespace flux

#endif // FLUX_ADAPTOR_DROP_HPP_INCLUDED
