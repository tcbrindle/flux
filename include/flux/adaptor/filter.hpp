
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_FILTER_HPP_INCLUDED
#define FLUX_ADAPTOR_FILTER_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/algorithm/find.hpp>

namespace flux {

namespace detail {

template <typename Base, typename Pred>
class filter_adaptor : public inline_sequence_base<filter_adaptor<Base, Pred>> {
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;

    template <typename BaseCtx, typename FilterFn>
    struct context_type : immovable {
        BaseCtx base_ctx;
        FilterFn filter_fn;

        using element_type = context_element_t<BaseCtx>;

        constexpr auto run_while(auto&& pred) -> iteration_result
        {
            return base_ctx.run_while([this, &pred](auto&& elem) {
                if (std::invoke(filter_fn, elem)) {
                    return std::invoke(pred, FLUX_FWD(elem));
                } else {
                    return loop_continue;
                }
            });
        }
    };

public:
    constexpr filter_adaptor(decays_to<Base> auto&& base, decays_to<Pred> auto&& pred)
        : base_(FLUX_FWD(base)),
          pred_(FLUX_FWD(pred))
    {}

    [[nodiscard]]
    constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]]
    constexpr auto base() && -> Base { return std::move(base_); }

    constexpr auto iterate()
    {
        return context_type{.base_ctx = flux::iterate(base_), .filter_fn = copy_or_ref(pred_)};
    }

    constexpr auto iterate() const
        requires iterable<Base const> && std::invocable<Pred&, iterable_element_t<Base const>>
    {
        return context_type{.base_ctx = flux::iterate(base_), .filter_fn = copy_or_ref(pred_)};
    }

    constexpr auto reverse_iterate()
        requires reverse_iterable<Base>
    {
        return context_type{.base_ctx = flux::reverse_iterate(base_),
                            .filter_fn = copy_or_ref(pred_)};
    }

    constexpr auto reverse_iterate() const
        requires reverse_iterable<Base const>
        && std::invocable<Pred&, iterable_element_t<Base const>>
    {
        return context_type{.base_ctx = flux::reverse_iterate(base_),
                            .filter_fn = copy_or_ref(pred_)};
    }

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool
                requires std::equality_comparable<cursor_t<Base>>
                = default;
        };

    public:
        using value_type = value_t<Base>;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;

        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type{flux::find_if(self.base_, std::ref(self.pred_))};
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.base_cur);
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at(self.base_, cur.base_cur))
        {
            return flux::read_at(self.base_, cur.base_cur);
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at_unchecked(self.base_, cur.base_cur))
        {
            return flux::read_at_unchecked(self.base_, cur.base_cur);
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at(self.base_, cur.base_cur))
        {
            return flux::move_at(self.base_, cur.base_cur);
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at_unchecked(self.base_, cur.base_cur))
        {
            return flux::move_at_unchecked(self.base_, cur.base_cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            flux::inc(self.base_, cur.base_cur);
            cur.base_cur = flux::slice(self.base_, std::move(cur).base_cur, flux::last)
                               .find_if(std::ref(self.pred_));
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
            requires bidirectional_sequence<Base>
        {
            do {
                flux::dec(self.base_, cur.base_cur);
            } while(!std::invoke(self.pred_, flux::read_at(self.base_, cur.base_cur)));
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            return cursor_type{flux::last(self.base_)};
        }

        static constexpr auto for_each_while(auto& self, auto&& func)
            -> cursor_type
        {
            return cursor_type {flux::seq_for_each_while(self.base_, [&](auto&& elem) {
                if (std::invoke(self.pred_, elem)) {
                    return std::invoke(func, FLUX_FWD(elem));
                } else {
                    return true;
                }
            })};
        }
    };
};

struct filter_fn {
    template <adaptable_iterable It, std::move_constructible Pred>
        requires std::predicate<Pred&, iterable_element_t<It> const&>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Pred pred) const
    {
        return filter_adaptor<std::decay_t<It>, Pred>(FLUX_FWD(it), std::move(pred));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto filter = detail::filter_fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::filter(Pred pred) &&
{
    return detail::filter_adaptor<D, Pred>(std::move(derived()), std::move(pred));
}


} // namespace flux

#endif // FLUX_ADAPTOR_FILTER_HPP_INCLUDED

