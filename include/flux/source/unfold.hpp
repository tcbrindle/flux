
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_UNFOLD_HPP_INCLUDED
#define FLUX_SOURCE_UNFOLD_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <typename R, typename Func>
struct unfold_sequence : inline_sequence_base<unfold_sequence<R, Func>> {
private:
    R state_;
    Func func_;

public:
    template <typename T>
        requires std::constructible_from<R, T>
    constexpr explicit unfold_sequence(Func&& func, T&& seed)
        : state_(FLUX_FWD(seed)),
          func_(std::move(func))
    {}

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            friend struct flux_sequence_traits;
            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;
        private:
            cursor_type() = default;
        };

        using self_t = unfold_sequence;

    public:
        static constexpr bool is_infinite = true;

        static constexpr auto first(self_t&) -> cursor_type { return {}; }

        static constexpr auto is_last(self_t&, cursor_type const&) -> bool { return false; }

        static constexpr auto inc(self_t& self, cursor_type&) -> void
        {
            self.state_ = std::invoke(self.func_, std::move(self.state_));
        }

        static constexpr auto read_at(self_t& self, cursor_type const&) -> R const&
        {
            return self.state_;
        }

        static constexpr auto for_each_while(self_t& self, auto&& pred) -> cursor_type
        {
            while (true) {
                if (!std::invoke(pred, self.state_)) {
                    break;
                }
                self.state_ = std::invoke(self.func_, std::move(self.state_));
            }

            return {};
        }
    };
};

struct unfold_fn {
    template <typename Func, typename Seed,
              typename R = std::decay_t<std::invoke_result_t<Func&, Seed>>>
        requires std::constructible_from<R, Seed> &&
                 std::invocable<Func&, R> &&
                 std::assignable_from<R&, std::invoke_result_t<Func&, R>>
    [[nodiscard]]
    constexpr auto operator()(Func func, Seed&& seed) const -> sequence auto
    {
        return unfold_sequence<R, Func>(std::move(func), FLUX_FWD(seed));
    }
};

} // namespace detail

inline constexpr auto unfold = detail::unfold_fn{};

} // namespace flux

#endif // FLUX_SOURCE_UNFOLD_INCLUDED
