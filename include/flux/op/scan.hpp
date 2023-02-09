
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SCAN_HPP_INCLUDED
#define FLUX_OP_SCAN_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/fold.hpp>

namespace flux {

namespace detail {

enum class scan_mode {
    inclusive,
    exclusive
};

template <typename Base, typename Func, typename R, scan_mode Mode>
struct scan_adaptor : inline_sequence_base<scan_adaptor<Base, Func, R, Mode>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;
    FLUX_NO_UNIQUE_ADDRESS R accum_;

public:
    constexpr scan_adaptor(decays_to<Base> auto&& base, Func&& func, auto&& init)
        : base_(FLUX_FWD(base)),
          func_(std::move(func)),
          accum_(FLUX_FWD(init))
    {}

    scan_adaptor(scan_adaptor&&) = default;
    scan_adaptor& operator=(scan_adaptor&&) = default;

    struct flux_sequence_traits {
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
            }
            return cursor_type(std::move(cur));
        }

        static constexpr auto is_last(self_t& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.base_cur);
        }

        static constexpr auto inc(self_t& self, cursor_type& cur) -> void
        {
            if constexpr (Mode == scan_mode::inclusive) {
                flux::inc(self.base_, cur.base_cur);
                update(self, cur.base_cur);
            } else {
                update(self, cur.base_cur);
                flux::inc(self.base_, cur.base_cur);
            }
        }

        static constexpr auto read_at(self_t& self, cursor_type const&) -> R const&
        {
            return self.accum_;
        }

        static constexpr auto last(self_t& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            return cursor_type(flux::last(self.base_));
        }

        static constexpr auto size(self_t& self) -> distance_t
            requires sized_sequence<Base>
        {
            return flux::size(self.base_);
        }

        static constexpr auto for_each_while(self_t& self, auto&& pred) -> cursor_type
        {
            return cursor_type(flux::for_each_while(self.base_, [&](auto&& elem) {
                if constexpr (Mode == scan_mode::inclusive) {
                    self.accum_ = std::invoke(self.func_, std::move(self.accum_), FLUX_FWD(elem));
                    return std::invoke(pred, std::as_const(self.accum_));
                } else {
                    if (std::invoke(pred, std::as_const(self.accum_))) {
                        self.accum_ = std::invoke(self.func_, std::move(self.accum_), FLUX_FWD(elem));
                        return true;
                    } else {
                        return false;
                    }
                }
            }));
        }
    };
};

struct scan_fn {
    template <adaptable_sequence Seq, typename Func, std::movable Init = value_t<Seq>,
              typename R = fold_result_t<Seq, Func, Init>>
        requires foldable<Seq, Func, Init>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func, Init init = Init{}) const
        -> sequence auto
    {
        return scan_adaptor<std::decay_t<Seq>, Func, R, scan_mode::inclusive>(
            FLUX_FWD(seq), std::move(func), std::move(init));
    }
};

struct exclusive_scan_fn {
    template <adaptable_sequence Seq, typename Func, std::movable Init,
              typename R = fold_result_t<Seq, Func, Init>>
        requires foldable<Seq, Func, Init>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func, Init init) const
        -> sequence auto
    {
        return scan_adaptor<std::decay_t<Seq>, Func, R, scan_mode::exclusive>(
            FLUX_FWD(seq), std::move(func), std::move(init));
    }
};

} // namespace detail

inline constexpr auto scan = detail::scan_fn{};
inline constexpr auto exclusive_scan = detail::exclusive_scan_fn{};

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
constexpr auto inline_sequence_base<Derived>::exclusive_scan(Func func, Init init) &&
{
    return flux::exclusive_scan(std::move(derived()), std::move(func), std::move(init));
}

} // namespace flux

#endif // FLUX_OP_SCAN_HPP_INCLUDED
