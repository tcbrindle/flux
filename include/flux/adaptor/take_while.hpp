
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_TAKE_WHILE_HPP_INCLUDED
#define FLUX_ADAPTOR_TAKE_WHILE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <typename Base, typename Pred>
struct take_while_adaptor : inline_sequence_base<take_while_adaptor<Base, Pred>> {
private:
    Base base_;
    Pred pred_;

    constexpr auto base() & -> Base& { return base_; }

    friend struct sequence_traits<take_while_adaptor>;
    friend struct passthrough_traits_base;

    template <typename BaseCtx, typename TakePred>
    struct context_type : immovable {
        BaseCtx base_ctx;
        TakePred take_pred;
        bool done = false;

        using element_type = context_element_t<BaseCtx>;

        constexpr auto run_while(auto&& pred) -> iteration_result
        {
            if (!done) {
                auto res = base_ctx.run_while([&](auto&& elem) {
                    if (!std::invoke(take_pred, std::as_const(elem))) {
                        done = true;
                        return loop_break;
                    } else {
                        return std::invoke(pred, FLUX_FWD(elem));
                    }
                });
                return static_cast<iteration_result>(static_cast<bool>(res) || done);
            } else {
                return iteration_result::complete;
            }
        }
    };

public:
    constexpr take_while_adaptor(decays_to<Base> auto&& base, decays_to<Pred> auto&& pred)
        : base_(FLUX_FWD(base)),
          pred_(FLUX_FWD(pred))
    {
    }

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base { return std::move(base_); }

    [[nodiscard]]
    constexpr auto iterate()
    {
        return context_type{.base_ctx = flux::iterate(base_), .take_pred = copy_or_ref(pred_)};
    }

    [[nodiscard]]
    constexpr auto iterate() const
        requires iterable<Base const>
        && std::predicate<Pred&, iterable_element_t<Base const> const&>
    {
        return context_type{.base_ctx = flux::iterate(base_), .take_pred = copy_or_ref(pred_)};
    }
};

struct take_while_fn {
    template <adaptable_iterable It, std::move_constructible Pred>
        requires std::predicate<Pred&, iterable_element_t<It> const&>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Pred pred) const
    {
        return take_while_adaptor<std::decay_t<It>, Pred>(FLUX_FWD(it), std::move(pred));
    }
};

} // namespace detail

template <typename Base, typename Pred>
struct sequence_traits<detail::take_while_adaptor<Base, Pred>> : detail::passthrough_traits_base {
    using self_t = detail::take_while_adaptor<Base, Pred>;

    using value_type = value_t<Base>;

    static constexpr bool is_infinite = false;

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
        return flux::seq_for_each_while(self.base_, [&](auto&& elem) {
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
constexpr auto inline_sequence_base<D>::take_while(Pred pred) &&
{
    return flux::take_while(std::move(derived()), std::move(pred));
}

} // namespace flux

#endif // FLUX_ADAPTOR_TAKE_WHILE_HPP_INCLUDED
