
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_MAP_HPP_INCLUDED
#define FLUX_ADAPTOR_MAP_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <iterable Base, typename Func>
struct map_adaptor : inline_sequence_base<map_adaptor<Base, Func>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

    friend struct iter_traits<map_adaptor>;

public:
    constexpr map_adaptor(decays_to<Base> auto&& base, decays_to<Func> auto&& func)
        : base_(FLUX_FWD(base)),
          func_(FLUX_FWD(func))
    {}

    constexpr auto base() & -> Base& { return base_; }
    constexpr auto base() const& -> Base const& { return base_; }
    constexpr auto base() && -> Base&& { return std::move(base_); }
    constexpr auto base() const&& -> Base const&& { return std::move(base_); }

    struct flux_iter_traits  : detail::passthrough_traits_base
    {
        using value_type = std::remove_cvref_t<std::invoke_result_t<Func&, element_t<Base>>>;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;
        static constexpr bool is_infinite = infinite_sequence<Base>;

        template <typename Self>
        static consteval auto element_type(Self& self)
            -> std::invoke_result_t<Func const&, element_t<decltype((self.base_))>>;

        static constexpr auto iterate(auto& self, auto&& pred) -> bool
        {
            return flux::iterate(self.base_, [&](auto&& elem) {
                return std::invoke(pred, std::invoke(self.func_, FLUX_FWD(elem)));
            });
        }

        template <typename Self>
        static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
            -> decltype(std::invoke(self.func_, flux::read_at(self.base_, cur)))
        {
            return std::invoke(self.func_, flux::read_at(self.base_, cur));
        }

        template <typename Self>
        static constexpr auto read_at_unchecked(Self& self, cursor_t<Self> const& cur)
            -> decltype(std::invoke(self.func_, flux::read_at_unchecked(self.base_, cur)))
        {
            return std::invoke(self.func_, flux::read_at_unchecked(self.base_, cur));
        }

        static constexpr auto for_each_while(auto& self, auto&& pred)
        {
            return flux::for_each_while(self.base_, [&](auto&& elem) {
                return std::invoke(pred, std::invoke(self.func_, FLUX_FWD(elem)));
            });
        }

        using default_iter_traits::move_at;
        using default_iter_traits::move_at_unchecked;

        static void data() = delete; // we're not a contiguous sequence
    };
};

struct map_fn {
    template <sink_iterable It, typename Func>
        requires std::regular_invocable<Func&, element_t<It>>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Func func) const
    {
        return map_adaptor<std::decay_t<It>, Func>(FLUX_FWD(it), std::move(func));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto map = detail::map_fn{};

template <typename Derived>
template <typename Func>
    requires std::invocable<Func&, element_t<Derived>>
constexpr auto inline_sequence_base<Derived>::map(Func func) &&
{
    return detail::map_adaptor<Derived, Func>(std::move(derived()), std::move(func));
}

} // namespace flux

#endif // FLUX_ADAPTOR_MAP_HPP_INCLUDED
