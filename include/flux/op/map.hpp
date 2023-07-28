
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_MAP_HPP_INCLUDED
#define FLUX_OP_MAP_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>

namespace flux {

namespace detail {

template <sequence Base, typename Func>
    requires std::is_object_v<Func> &&
             std::regular_invocable<Func&, element_t<Base>>
struct map_adaptor : inline_sequence_base<map_adaptor<Base, Func>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

    friend struct sequence_traits<map_adaptor>;

public:
    constexpr map_adaptor(decays_to<Base> auto&& base, decays_to<Func> auto&& func)
        : base_(FLUX_FWD(base)),
          func_(FLUX_FWD(func))
    {}

    constexpr auto base() & -> Base& { return base_; }
    constexpr auto base() const& -> Base const& { return base_; }
    constexpr auto base() && -> Base&& { return std::move(base_); }
    constexpr auto base() const&& -> Base const&& { return std::move(base_); }

    struct flux_sequence_traits  : detail::passthrough_traits_base<Base>
    {
        using value_type = std::remove_cvref_t<std::invoke_result_t<Func&, element_t<Base>>>;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;
        static constexpr bool is_infinite = infinite_sequence<Base>;

        template <typename Self>
        static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
            -> decltype(std::invoke(self.func_, flux::read_at(self.base_, cur)))
        {
            return std::invoke(self.func_, flux::read_at(self.base_, cur));
        }

        static constexpr auto for_each_while(auto& self, auto&& pred)
        {
            return flux::for_each_while(self.base_, [&](auto&& elem) {
                return std::invoke(pred, std::invoke(self.func_, FLUX_FWD(elem)));
            });
        }

        static void move_at() = delete; // Use the base version of move_at
        static void data() = delete; // we're not a contiguous sequence
    };
};

struct map_fn {
    template <adaptable_sequence Seq, typename Func>
        requires std::regular_invocable<Func&, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func) const
    {
        return map_adaptor<std::decay_t<Seq>, Func>(FLUX_FWD(seq), std::move(func));
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

#endif
