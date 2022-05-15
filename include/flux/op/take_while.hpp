
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_TAKE_WHILE_HPP_INCLUDED
#define FLUX_OP_TAKE_WHILE_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>

namespace flux {

namespace detail {

template <lens Base, typename Pred>
struct take_while_adaptor : lens_base<take_while_adaptor<Base, Pred>> {
private:
    Base base_;
    Pred pred_;

    constexpr auto base() & -> Base& { return base_; }

    friend struct sequence_iface<take_while_adaptor>;
    friend struct passthrough_iface_base<Base>;

public:
    constexpr take_while_adaptor(Base&& base, Pred&& pred)
        : base_(std::move(base)),
          pred_(std::move(pred))
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base { return std::move(base_); }
};

struct take_while_fn {
    template <adaptable_sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>&>
    constexpr auto operator()(Seq&& seq, Pred pred) const
    {
        return take_while_adaptor(flux::from(seq), std::move(pred));
    }
};

} // namespace detail

template <typename Base, typename Pred>
struct sequence_iface<detail::take_while_adaptor<Base, Pred>>
    : detail::passthrough_iface_base<Base>
{
    using self_t = detail::take_while_adaptor<Base, Pred>;

    static constexpr bool is_infinite = false;

    static constexpr bool is_last(self_t& self, cursor_t<Base> const& idx)
    {
        if (flux::is_last(self.base_, idx) ||
            !std::invoke(self.pred_, flux::read_at(self.base_, idx))) {
            return true;
        } else {
            return false;
        }
    }

    static constexpr bool is_last(self_t const& self, cursor_t<Base const> const& cur)
        requires std::predicate<Pred const&, element_t<Base const>>
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

inline constexpr auto take_while = detail::take_while_fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto lens_base<D>::take_while(Pred pred) &&
{
    return flux::take_while(std::move(derived()), std::move(pred));
}

} // namespace flux

#endif
