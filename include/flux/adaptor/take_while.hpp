
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_TAKE_WHILE_HPP_INCLUDED
#define FLUX_ADAPTOR_TAKE_WHILE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <iterable Base, typename Pred>
struct take_while_adaptor : inline_iter_base<take_while_adaptor<Base, Pred>> {
private:
    Base base_;
    Pred pred_;

    constexpr auto base() & -> Base& { return base_; }

    friend struct iter_traits<take_while_adaptor>;
    friend struct passthrough_traits_base;

public:
    constexpr take_while_adaptor(decays_to<Base> auto&& base, decays_to<Pred> auto&& pred)
        : base_(FLUX_FWD(base)),
          pred_(FLUX_FWD(pred))
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base { return std::move(base_); }
};

struct take_while_fn {
    template <sink_iterable It, std::move_constructible Pred>
        requires std::predicate<Pred&, element_t<It>>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Pred pred) const
    {
        return take_while_adaptor<std::decay_t<It>, Pred>(
                    FLUX_FWD(it), std::move(pred));
    }
};

} // namespace detail

template <iterable Base, typename Pred>
struct iter_traits<detail::take_while_adaptor<Base, Pred>>
    : default_iter_traits {

    template <typename Self>
    static consteval auto element_type(Self& self)
        -> element_t<decltype((self.base_))>;

    static constexpr auto iterate(auto& self, auto&& iter_pred) -> bool
    {
        bool done = false;
        bool res = flux::iterate(self.base_, [&](auto&& elem) {
            if (!std::invoke(self.pred_, elem)) {
                done = true;
                return false; // break
            } else {
                return std::invoke(iter_pred, FLUX_FWD(elem));
            }
        });
        // Return true if take_while was exhausted
        return res ? res : done;
    }

};

template <sequence Base, typename Pred>
struct iter_traits<detail::take_while_adaptor<Base, Pred>>
    : detail::passthrough_traits_base
{
    using self_t = detail::take_while_adaptor<Base, Pred>;

    using value_type = value_t<Base>;

    static constexpr bool is_infinite = false;

    using default_iter_traits::element_type;

    static constexpr auto iterate(auto& self, auto&& iter_pred) -> bool
    {
        bool done = false;
        bool res = flux::iterate(self.base_, [&](auto&& elem) {
            if (!std::invoke(self.pred_, elem)) {
                done = true;
                return false; // break
            } else {
                return std::invoke(iter_pred, FLUX_FWD(elem));
            }
        });
        // Return true if take_while was exhausted
        return res ? res : done;
    }

    template <typename Self>
    static constexpr bool is_last(Self& self, cursor_t<Self> const& cur)
        requires std::predicate<decltype((self.pred_)), element_t<decltype(self.base_)>>
    {
        if (flux::is_last(self.base_, cur) ||
            !std::invoke(self.pred_, flux::read_at(self.base_, cur))) {
            return true;
        } else {
            return false;
        }
    }

    void last() = delete;
    void size() = delete;

    static constexpr auto for_each_while(auto& self, auto&& func)
    {
        return flux::for_each_while(self.base_, [&](auto&& elem) {
            if (!std::invoke(self.pred_, elem)) {
                return false;
            } else {
                return std::invoke(func, FLUX_FWD(elem));
            }
        });
    }
};

FLUX_EXPORT inline constexpr auto take_while = detail::take_while_fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_iter_base<D>::take_while(Pred pred) &&
{
    return flux::take_while(std::move(derived()), std::move(pred));
}

} // namespace flux

#endif // FLUX_ADAPTOR_TAKE_WHILE_HPP_INCLUDED
