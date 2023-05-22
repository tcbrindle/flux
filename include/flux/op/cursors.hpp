
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CURSORS_HPP_INCLUDED
#define FLUX_OP_CURSORS_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <typename Base>
struct cursors_adaptor : inline_sequence_base<cursors_adaptor<Base>> {
private:
    Base base_;

public:
    constexpr explicit cursors_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_sequence_traits {

        static inline constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto first(auto& self) -> decltype(flux::first(self.base_))
        {
            return flux::first(self.base_);
        }

        static constexpr auto is_last(auto& self, cursor_t<Base> const& cur) -> bool
        {
            return flux::is_last(self.base_, cur);
        }

        static constexpr auto inc(auto& self, cursor_t<Base>& cur) -> void
        {
            flux::inc(self.base_, cur);
        }

        static constexpr auto read_at(auto&, cursor_t<Base> const& cur)
            -> cursor_t<Base>
        {
            return cur;
        }

        static constexpr auto dec(auto& self, cursor_t<Base>& cur) -> void
            requires bidirectional_sequence<Base>
        {
            flux::dec(self.base_, cur);
        }

        static constexpr auto inc(auto& self, cursor_t<Base>& cur, distance_t offset) -> void
            requires random_access_sequence<Base>
        {
            flux::inc(self.base_, cur, offset);
        }

        static constexpr auto distance(auto& self, cursor_t<Base> const& from,
                                       cursor_t<Base> const& to) -> distance_t
            requires random_access_sequence<Base>
        {
            return flux::distance(self.base_, from, to);
        }

        static constexpr auto last(auto& self) -> cursor_t<Base>
            requires bounded_sequence<Base>
        {
            return flux::last(self.base_);
        }

        static constexpr auto size(auto& self) -> distance_t
            requires sized_sequence<Base>
        {
            return flux::size(self.base_);
        }
    };
};

struct cursors_fn {
    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> sequence auto
    {
        return cursors_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
    }
};

} // namespace detail

inline constexpr auto cursors = detail::cursors_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::cursors() &&
    requires multipass_sequence<D>
{
    return flux::cursors(std::move(derived()));
}

} // namespace flux

#endif // FLUX_OP_CURSORS_HPP_INCLUDED
