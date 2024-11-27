
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_UNCHECKED_HPP_INCLUDED
#define FLUX_ADAPTOR_UNCHECKED_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <sequence Base>
struct unchecked_adaptor : inline_iter_base<unchecked_adaptor<Base>> {
private:
    Base base_;

public:
    constexpr explicit unchecked_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    constexpr auto base() & -> Base& { return base_; }
    constexpr auto base() const& -> Base const& { return base_; }

    struct flux_iter_traits : passthrough_traits_base {

        using value_type = value_t<Base>;
        static constexpr bool disable_multipass = !multipass_sequence<Base>;
        static constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto read_at(auto& self, auto const& cur)
            -> element_t<Base>
        {
            return flux::read_at_unchecked(self.base(), cur);
        }

        static constexpr auto move_at(auto& self, auto const& cur)
            -> rvalue_element_t<Base>
        {
            return flux::move_at_unchecked(self.base(), cur);
        }
    };
};

struct unchecked_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
        -> unchecked_adaptor<std::decay_t<Seq>>
    {
        return unchecked_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto unchecked = detail::unchecked_fn{};

} // namespace flux

#endif // FLUX_ADAPTOR_UNCHECKED_HPP_INCLUDED
