
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_SCAN_HPP_INCLUDED
#define FLUX_ADAPTOR_SCAN_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/algorithm/fold.hpp>

#include <utility> // for std::as_const

namespace flux {

namespace detail {

enum class scan_mode {
    inclusive,
    exclusive
};

template <scan_mode>
struct scan_cursor_base {};

template <>
struct scan_cursor_base<scan_mode::exclusive> {
    bool is_last;
};

template <typename Base, typename Func, typename R, scan_mode Mode>
struct scan_adaptor : inline_sequence_base<scan_adaptor<Base, Func, R, Mode>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;
    FLUX_NO_UNIQUE_ADDRESS R accum_;

    struct context_type : immovable {
        scan_adaptor* parent;
        iteration_context_t<Base> base_ctx;
        bool first = true;

        using element_type = R const&;

        constexpr auto run_while(auto&& pred) -> iteration_result
        {
            if constexpr (Mode == scan_mode::exclusive) {
                if (first) {
                    first = false;
                    if (!pred(std::as_const(parent->accum_))) {
                        return iteration_result::incomplete;
                    }
                }
            }
            return base_ctx.run_while([&](auto&& elem) {
                parent->accum_
                    = std::invoke(parent->func_, std::move(parent->accum_), FLUX_FWD(elem));
                return pred(std::as_const(parent->accum_));
            });
        }
    };

public:
    constexpr scan_adaptor(decays_to<Base> auto&& base, Func&& func, auto&& init)
        : base_(FLUX_FWD(base)),
          func_(std::move(func)),
          accum_(FLUX_FWD(init))
    {}

    scan_adaptor(scan_adaptor&&) = default;
    scan_adaptor& operator=(scan_adaptor&&) = default;

    constexpr auto iterate() -> context_type
    {
        return context_type{.parent = this, .base_ctx = flux::iterate(base_)};
    }

    constexpr auto size() -> int_t
        requires sized_iterable<Base>
    {
        if constexpr (Mode == scan_mode::exclusive) {
            return num::add(flux::iterable_size(base_), int_t{1});
        } else {
            return flux::iterable_size(base_);
        }
    }

    struct flux_sequence_traits : default_sequence_traits {
    private:

        struct cursor_type : private scan_cursor_base<Mode> {
            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;

        private:
            friend struct flux_sequence_traits;

            constexpr explicit cursor_type(cursor_t<Base>&& base_cur)
                : base_cur(std::move(base_cur))
            {}

            cursor_t<Base> base_cur;
        };

        using self_t = scan_adaptor;

        static constexpr auto update(self_t& self, cursor_t<Base> const& cur) -> void
        {
            if (!flux::is_last(self.base_, cur)) {
                self.accum_ = std::invoke(self.func_, std::move(self.accum_),
                                          flux::read_at(self.base_, cur));
            }
        }

    public:
        static inline constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto first(self_t& self) -> cursor_type
        {
            auto cur = flux::first(self.base_);
            if constexpr (Mode == scan_mode::inclusive) {
                update(self, cur);
                return cursor_type(std::move(cur));
            } else if constexpr (Mode == scan_mode::exclusive) {
                bool last = flux::is_last(self.base_, cur);
                cursor_type out = cursor_type(std::move(cur));
                out.is_last = last;
                return out;
            }
        }

        static constexpr auto is_last(self_t& self, cursor_type const& cur) -> bool
        {
            if constexpr (Mode == scan_mode::exclusive) {
                return cur.is_last;
            } else {
                return flux::is_last(self.base_, cur.base_cur);
            }
        }

        static constexpr auto inc(self_t& self, cursor_type& cur) -> void
        {
            if constexpr (Mode == scan_mode::inclusive) {
                flux::inc(self.base_, cur.base_cur);
                update(self, cur.base_cur);
            } else {
                update(self, cur.base_cur);
                if (flux::is_last(self.base_, cur.base_cur)) {
                    cur.is_last = true;
                } else {
                    flux::inc(self.base_, cur.base_cur);
                }
            }
        }

        static constexpr auto read_at(self_t& self, cursor_type const&) -> R const&
        {
            return self.accum_;
        }

        static constexpr auto last(self_t& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            auto cur = cursor_type(flux::last(self.base_));
            if constexpr (Mode == scan_mode::exclusive) {
                cur.is_last = true;
            }
            return cur;
        }

        static constexpr auto size(self_t& self) -> int_t
            requires sized_sequence<Base>
        {
            if constexpr (Mode == scan_mode::exclusive) {
                return num::add(flux::size(self.base_), int_t {1});
            } else {
                return flux::size(self.base_);
            }
        }

        static constexpr auto for_each_while(self_t& self, auto&& pred) -> cursor_type
            requires (Mode != scan_mode::exclusive)
        {
            return cursor_type(flux::seq_for_each_while(self.base_, [&](auto&& elem) {
                self.accum_ = std::invoke(self.func_, std::move(self.accum_), FLUX_FWD(elem));
                return std::invoke(pred, std::as_const(self.accum_));
            }));
        }

        using default_sequence_traits::for_each_while; // when Mode == exclusive
    };
};

struct scan_fn {
    template <adaptable_iterable It, typename Func, std::movable Init = iterable_value_t<It>,
              typename R = fold_result_t<It, Func, Init>>
        requires foldable<It, Func, Init>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Func func, Init init = Init{}) const -> iterable auto
    {
        return scan_adaptor<std::decay_t<It>, Func, R, scan_mode::inclusive>(
            FLUX_FWD(it), std::move(func), std::move(init));
    }
};

struct prescan_fn {
    template <adaptable_iterable It, typename Func, std::movable Init,
              typename R = fold_result_t<It, Func, Init>>
        requires foldable<It, Func, Init>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Func func, Init init) const -> iterable auto
    {
        return scan_adaptor<std::decay_t<It>, Func, R, scan_mode::exclusive>(
            FLUX_FWD(it), std::move(func), std::move(init));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto scan = detail::scan_fn{};
FLUX_EXPORT inline constexpr auto prescan = detail::prescan_fn{};

template <typename Derived>
template <typename D, typename Func, typename Init>
    requires foldable<Derived, Func, Init>
constexpr auto inline_sequence_base<Derived>::scan(Func func, Init init) &&
{
    return flux::scan(std::move(derived()), std::move(func), std::move(init));
}

template <typename Derived>
template <typename Func, typename Init>
    requires foldable<Derived, Func, Init>
constexpr auto inline_sequence_base<Derived>::prescan(Func func, Init init) &&
{
    return flux::prescan(std::move(derived()), std::move(func), std::move(init));
}

} // namespace flux

#endif // FLUX_ADAPTOR_SCAN_HPP_INCLUDED
