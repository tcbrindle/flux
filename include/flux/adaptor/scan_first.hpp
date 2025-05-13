
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

public:
    constexpr scan_first_adaptor(decays_to<Base> auto&& base, Func&& func)
        : base_(FLUX_FWD(base)),
          func_(std::move(func))
    {}

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
    template <adaptable_sequence Seq, typename Func>
        requires foldable<Seq, Func, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func) const -> sequence auto
    {
        using R = fold_result_t<Seq, Func, element_t<Seq>>;
        return scan_first_adaptor<std::decay_t<Seq>, Func, R>(
            FLUX_FWD(seq), std::move(func));
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
