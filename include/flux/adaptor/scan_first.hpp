
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_SCAN_FIRST_HPP_INCLUDED
#define FLUX_ADAPTOR_SCAN_FIRST_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/algorithm/fold.hpp>

#include <utility> // for std::as_const

namespace flux {

namespace detail {

template <typename Base, typename Func, typename R>
struct scan_first_adaptor : inline_sequence_base<scan_first_adaptor<Base, Func, R>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;
    flux::optional<R> accum_;

    struct context_type : immovable {
        scan_first_adaptor* parent;
        iteration_context_t<Base> base_ctx;

        using element_type = R const&;

        constexpr auto run_while(auto&& pred) -> iteration_result
        {
            if (!parent->accum_.has_value()) {
                base_ctx.run_while([&](auto&& elem) {
                    parent->accum_.emplace(FLUX_FWD(elem));
                    return false;
                });
                if (!parent->accum_.has_value()) {
                    return iteration_result::complete;
                }
                if (!std::invoke(pred, std::as_const(*parent->accum_))) {
                    return iteration_result::incomplete;
                }
            }

            FLUX_DEBUG_ASSERT(parent->accum_.has_value());
            return base_ctx.run_while([&](auto&& elem) {
                parent->accum_.emplace(
                    std::invoke(parent->func_, *std::move(parent->accum_), FLUX_FWD(elem)));
                return std::invoke(pred, std::as_const(*parent->accum_));
            });
        }
    };

public:
    constexpr scan_first_adaptor(decays_to<Base> auto&& base, Func&& func)
        : base_(FLUX_FWD(base)),
          func_(std::move(func))
    {
    }

    scan_first_adaptor(scan_first_adaptor&&) = default;
    scan_first_adaptor& operator=(scan_first_adaptor&&) = default;

    [[nodiscard]] constexpr auto iterate() -> context_type
    {
        return context_type{.parent = this, .base_ctx = flux::iterate(base_)};
    }

    [[nodiscard]] constexpr auto size() -> int_t
        requires sized_iterable<Base>
    {
        return flux::iterable_size(base_);
    }

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;

        private:
            friend struct flux_sequence_traits;

            constexpr explicit cursor_type(cursor_t<Base>&& base_cur)
                : base_cur(std::move(base_cur))
            {}

            cursor_t<Base> base_cur;
        };

        using self_t = scan_first_adaptor;

    public:
        static constexpr auto first(self_t& self) -> cursor_type
        {
            auto cur = flux::first(self.base_);
            if (!flux::is_last(self.base_, cur)) {
                self.accum_.emplace(flux::read_at(self.base_, cur));
            }
            return cursor_type(std::move(cur));
        }

        static constexpr auto is_last(self_t& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.base_cur);
        }

        static constexpr auto inc(self_t& self, cursor_type& cur) -> void
        {
            flux::inc(self.base_, cur.base_cur);
            if (!flux::is_last(self.base_, cur.base_cur)) {
                self.accum_.emplace(
                    std::invoke(self.func_,
                                std::move(self.accum_.value_unchecked()),
                                flux::read_at(self.base_, cur.base_cur)));
            }
        }

        static constexpr auto read_at(self_t& self, cursor_type const&) -> R const&
        {
            return self.accum_.value();
        }

        static constexpr auto read_at_unchecked(self_t& self, cursor_type const&)
            -> R const&
        {
            return self.accum_.value_unchecked();
        }

        static constexpr auto last(self_t& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            return cursor_type(flux::last(self.base_));
        }

        static constexpr auto size(self_t& self) -> int_t
            requires sized_sequence<Base>
        {
            return flux::size(self.base_);
        }

        static constexpr auto for_each_while(self_t& self, auto&& pred) -> cursor_type
        {
            return cursor_type(flux::seq_for_each_while(self.base_, [&](auto&& elem) {
                if (self.accum_.has_value()) {
                    self.accum_.emplace(
                        std::invoke(self.func_,
                                    std::move(self.accum_.value_unchecked()),
                                    FLUX_FWD(elem)));
                } else {
                    self.accum_.emplace(FLUX_FWD(elem));
                }
                return std::invoke(pred, self.accum_.value_unchecked());
            }));
        }
    };
};

struct scan_first_fn {
    template <adaptable_iterable It, typename Func>
        requires foldable<It, Func, iterable_element_t<It>>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Func func) const -> iterable auto
    {
        using R = fold_result_t<It, Func, iterable_element_t<It>>;
        return scan_first_adaptor<std::decay_t<It>, Func, R>(FLUX_FWD(it), std::move(func));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto scan_first = detail::scan_first_fn{};

template <typename Derived>
template <typename Func>
    requires foldable<Derived, Func, element_t<Derived>>
constexpr auto inline_sequence_base<Derived>::scan_first(Func func) &&
{
    return flux::scan_first(std::move(derived()), std::move(func));
}

} // namespace flux

#endif // FLUX_ADAPTOR_SCAN_FIRST_HPP_INCLUDED
