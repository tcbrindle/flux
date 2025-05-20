
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_DROP_WHILE_HPP_INCLUDED
#define FLUX_ADAPTOR_DROP_WHILE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <typename Base, typename Pred>
struct drop_while_adaptor : inline_sequence_base<drop_while_adaptor<Base, Pred>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;

    friend struct passthrough_traits_base;

    constexpr auto base() & -> Base& { return base_; }
    constexpr auto base() const& -> Base const& { return base_; }

    template <typename BaseCtx, typename DropPred>
    struct context_type : immovable {
        BaseCtx base_ctx;
        DropPred drop_pred;
        bool done = false;

        using element_type = context_element_t<BaseCtx>;

        constexpr auto run_while(auto&& pred) -> iteration_result
        {
            return base_ctx.run_while([&](auto&& elem) {
                if (done) {
                    return std::invoke(pred, FLUX_FWD(elem));
                } else {
                    if (std::invoke(drop_pred, std::as_const(elem))) {
                        return loop_continue;
                    } else {
                        done = true;
                        return std::invoke(pred, FLUX_FWD(elem));
                    }
                }
            });
        }
    };

public:
    constexpr drop_while_adaptor(decays_to<Base> auto&& base, decays_to<Pred> auto&& pred)
        : base_(FLUX_FWD(base)),
          pred_(FLUX_FWD(pred))
    {
    }

    [[nodiscard]] constexpr auto iterate()
    {
        return context_type{.base_ctx = flux::iterate(base_), .drop_pred = copy_or_ref(pred_)};
    }

    [[nodiscard]]
    constexpr auto iterate() const
        requires iterable<Base const> && std::predicate<Pred&, iterable_element_t<Base const>>
    {
        return context_type{.base_ctx = flux::iterate(base_), .drop_pred = copy_or_ref(pred_)};
    }

    struct flux_sequence_traits : detail::passthrough_traits_base {
        static constexpr bool disable_multipass = !multipass_sequence<Base>;

        static constexpr auto first(auto& self)
            requires sequence<decltype((self.base_))>
        {
            return flux::seq_for_each_while(self.base_, std::ref(self.pred_));
        }

        static constexpr auto data(auto& self)
            requires contiguous_sequence<Base>
        {
            return flux::data(self.base_) +
                   flux::distance(self.base_, flux::first(self.base_), first(self));
        }

        using default_sequence_traits::size;
        using default_sequence_traits::for_each_while;
    };
};

struct drop_while_fn {
    template <adaptable_iterable It, std::move_constructible Pred>
        requires std::predicate<Pred&, iterable_element_t<It>>
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
constexpr auto inline_sequence_base<D>::drop_while(Pred pred) &&
{
    return flux::drop_while(std::move(derived()), std::move(pred));
};

} // namespace flux

#endif // FLUX_ADAPTOR_DROP_WHILE_HPP_INCLUDED
