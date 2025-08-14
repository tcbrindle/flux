
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SEQUENCE_UNFOLD_HPP_INCLUDED
#define FLUX_SEQUENCE_UNFOLD_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <typename R, typename Func>
struct unfold_iterable : inline_sequence_base<unfold_iterable<R, Func>> {
    R state_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

    struct context_type : immovable {
        using element_type = R const&;

        R* state_;
        std::reference_wrapper<Func> func_;
        bool inc_next_ = false;

        constexpr explicit context_type(unfold_iterable& self)
            : state_(std::addressof(self.state_)),
              func_(std::ref(self.func_))
        {
        }

        constexpr auto run_while(auto&& pred) -> iteration_result
        {
            if (inc_next_) {
                *state_ = std::invoke(func_, std::move(*state_));
            }
            while (true) {
                if (!std::invoke(pred, *state_)) {
                    inc_next_ = true;
                    return iteration_result::incomplete;
                };
                *state_ = std::invoke(func_, std::move(*state_));
            }
        }
    };

public:
    template <typename T>
        requires std::constructible_from<R, T>
    constexpr unfold_iterable(Func&& func, T&& seed)
        : state_(FLUX_FWD(seed)),
          func_(std::move(func))
    {
    }

    [[nodiscard]]
    constexpr auto iterate()
    {
        return context_type{*this};
    }
};

struct unfold_fn {
    template <typename Func, typename Seed,
              typename R = std::decay_t<std::invoke_result_t<Func&, Seed>>>
        requires std::constructible_from<R, Seed> && std::invocable<Func&, R>
        && std::assignable_from<R&, std::invoke_result_t<Func&, R>>
    [[nodiscard]]
    constexpr auto operator()(Func func, Seed&& seed) const -> iterable auto
    {
        return unfold_iterable<R, Func>(std::move(func), FLUX_FWD(seed));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto unfold = detail::unfold_fn{};

} // namespace flux

#endif // FLUX_SEQUENCE_UNFOLD_INCLUDED
