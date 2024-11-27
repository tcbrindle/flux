
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_DROP_WHILE_HPP_INCLUDED
#define FLUX_ADAPTOR_DROP_WHILE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <iterable Base, typename Pred>
struct drop_while_adaptor : inline_iter_base<drop_while_adaptor<Base, Pred>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;

    friend struct passthrough_traits_base;

    constexpr auto base() & -> Base& { return base_; }
    constexpr auto base() const& -> Base const& { return base_; }

public:
    constexpr drop_while_adaptor(decays_to<Base> auto&& base, decays_to<Pred> auto&& pred)
        : base_(FLUX_FWD(base)),
          pred_(FLUX_FWD(pred))
    {}

    struct flux_iter_traits : detail::passthrough_traits_base {
        using value_type = value_t<Base>;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;

        static constexpr auto iterate(auto& self, auto&& iter_pred) -> bool
        {
            bool found_first = false;
            return flux::iterate(self.base_, [&](auto&& elem) {
                if (!found_first) {
                    if (std::invoke(self.pred_, elem)) {
                        return true; // continue
                    } else {
                        found_first = true;
                    }
                }
                return std::invoke(iter_pred, FLUX_FWD(elem));
            });
        }

        static constexpr auto first(auto& self)
            requires sequence<decltype((self.base_))>
        {
            return flux::for_each_while(self.base_, std::ref(self.pred_));
        }

        static constexpr auto data(auto& self)
            requires contiguous_sequence<Base>
        {
            return flux::data(self.base_) +
                   flux::distance(self.base_, flux::first(self.base_), first(self));
        }

        using default_iter_traits::size;
        using default_iter_traits::for_each_while;
    };
};

struct drop_while_fn {
    template <sink_iterable It, std::move_constructible Pred>
        requires std::predicate<Pred&, element_t<It>>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Pred pred) const
    {
        return drop_while_adaptor<std::decay_t<It>, Pred>(FLUX_FWD(it), std::move(pred));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto drop_while = detail::drop_while_fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_iter_base<D>::drop_while(Pred pred) &&
{
    return flux::drop_while(std::move(derived()), std::move(pred));
};

} // namespace flux

#endif // FLUX_ADAPTOR_DROP_WHILE_HPP_INCLUDED
