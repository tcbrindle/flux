
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_DROP_HPP_INCLUDED
#define FLUX_ADAPTOR_DROP_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/adaptor/stride.hpp>

namespace flux {

namespace detail {

template <typename Base>
struct drop_adaptor : inline_sequence_base<drop_adaptor<Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    int_t count_;

    template <typename BaseCtx>
    struct context_type : immovable {
        BaseCtx base_ctx;
        int_t remaining;

        using element_type = context_element_t<BaseCtx>;

        constexpr auto run_while(auto&& pred) -> iteration_result
        {
            return base_ctx.run_while([&](auto&& elem) {
                if (remaining > 0) {
                    --remaining;
                    return loop_continue;
                } else {
                    return pred(FLUX_FWD(elem));
                }
            });
        }
    };

public:
    constexpr drop_adaptor(decays_to<Base> auto&& base, int_t count)
        : base_(FLUX_FWD(base)),
          count_(count)
    {}

    constexpr Base& base() & { return base_; }
    constexpr Base const& base() const& { return base_; }

    [[nodiscard]] constexpr auto iterate()
    {
        return context_type{.base_ctx = flux::iterate(base_), .remaining = count_};
    }

    [[nodiscard]] constexpr auto iterate() const
        requires iterable<Base const>
    {
        return context_type{.base_ctx = flux::iterate(base_), .remaining = count_};
    }

    [[nodiscard]] constexpr auto size() -> int_t
        requires sized_iterable<Base>
    {
        auto sz = flux::iterable_size(base_);
        return (cmp::max)(num::sub(sz, count_), int_t{0});
    }

    [[nodiscard]] constexpr auto size() const -> int_t
        requires sized_iterable<Base const>
    {
        auto sz = flux::iterable_size(base_);
        return (cmp::max)(num::sub(sz, count_), int_t{0});
    }

    struct flux_sequence_traits : passthrough_traits_base {
        using value_type = value_t<Base>;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;

        static constexpr auto first(auto& self) -> cursor_t<Base>
        {
            auto cur = flux::first(self.base_);
            detail::advance(self.base_, cur, self.count_);
            return cur;
        }

        static constexpr auto size(auto& self)
            requires sized_sequence<Base>
        {
            return (cmp::max)(num::sub(flux::size(self.base()), self.count_), int_t {0});
        }

        static constexpr auto data(auto& self)
            requires contiguous_sequence<Base> && sized_sequence<Base>
        {
            return flux::data(self.base()) + (cmp::min)(self.count_, flux::size(self.base_));
        }

        static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_t<Base>
        {
            return default_sequence_traits::for_each_while(self, FLUX_FWD(pred));
        }
    };
};

struct drop_fn {
    template <adaptable_iterable It>
    [[nodiscard]]
    constexpr auto operator()(It&& it, num::integral auto count) const
    {
        auto count_ = num::checked_cast<int_t>(count);
        if (count_ < 0) {
            runtime_error("Negative argument passed to drop()");
        }

        return drop_adaptor<std::decay_t<It>>(FLUX_FWD(it), count_);
    }

};

} // namespace detail

FLUX_EXPORT inline constexpr auto drop = detail::drop_fn{};

template <typename Derived>
constexpr auto inline_sequence_base<Derived>::drop(num::integral auto count) &&
{
    return flux::drop(std::move(derived()), count);
}

} // namespace flux

#endif // FLUX_ADAPTOR_DROP_HPP_INCLUDED
