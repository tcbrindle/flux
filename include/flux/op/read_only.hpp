
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_READ_ONLY_HPP_INCLUDED
#define FLUX_OP_READ_ONLY_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/map.hpp>

namespace flux {

namespace detail {

template <sequence Base>
    requires (not read_only_sequence<Base>)
struct read_only_adaptor : inline_sequence_base<read_only_adaptor<Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;

public:
    constexpr read_only_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_sequence_traits {
    private:
        using const_rvalue_element_t = std::common_reference_t<
            value_t<Base> const&&, rvalue_element_t<Base>>;

    public:
        using value_type = value_t<Base>;

        static constexpr auto first(auto& self) -> cursor_t<Base> { return flux::first(self.base_); }

        static constexpr auto is_last(auto& self, cursor_t<Base> const& cur) -> bool
        {
            return flux::is_last(self.base_,  cur);
        }

        static constexpr auto inc(auto& self, cursor_t<Base>& cur) -> void
        {
            flux::inc(self.base_, cur);
        }

        static constexpr auto read_at(auto& self, cursor_t<Base> const& cur)
            -> const_element_t<Base>
        {
            return static_cast<const_element_t<Base>>(flux::read_at(self.base_, cur));
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_t<Base> const& cur)
            -> const_element_t<Base>
        {
            return static_cast<const_element_t<Base>>(flux::read_at_unchecked(self.base_, cur));
        }

        static constexpr auto move_at(auto& self, cursor_t<Base> const& cur)
            -> const_rvalue_element_t
        {
            return static_cast<const_rvalue_element_t>(flux::move_at(self.base_, cur));
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_t<Base> const& cur)
            -> const_rvalue_element_t
        {
            return static_cast<const_rvalue_element_t>(flux::move_at_unchecked(self.base_, cur));
        }

        static constexpr auto last(auto& self) -> cursor_t<Base>
            requires bounded_sequence<Base>
        {
            return flux::last(self.base_);
        }

        static constexpr auto dec(auto& self, cursor_t<Base>& cur)
            requires bidirectional_sequence<Base>
        {
            return flux::dec(self.base_, cur);
        }

        static constexpr auto inc(auto& self, cursor_t<Base>& cur, distance_t o)
            requires random_access_sequence<Base>
        {
            return flux::inc(self.base_, cur, o);
        }

        static constexpr auto distance(auto& self, cursor_t<Base> const& from,
                                       cursor_t<Base> const& to) -> distance_t
            requires random_access_sequence<Base>
        {
            return flux::distance(self.base_, from, to);
        }

        static constexpr auto size(auto& self) -> distance_t
            requires sized_sequence<Base>
        {
            return flux::size(self.base_);
        }

        static constexpr auto data(auto& self)
            requires contiguous_sequence<Base>
        {
            using P = std::add_pointer_t<std::remove_reference_t<const_element_t<Base>>>;
            return static_cast<P>(flux::data(self.base_));
        }

        static constexpr auto for_each_while(auto& self, auto&& pred)
        {
            return flux::for_each_while(self.base_, [&pred](auto&& elem) {
                return std::invoke(pred, static_cast<const_element_t<Base>>(FLUX_FWD(elem)));
            });
        }
    };


};

struct read_only_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr (read_only_sequence<Seq>) {
            return FLUX_FWD(seq);
        } else {
            return read_only_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
        }
    }
};


} // namespace detail

inline constexpr auto read_only = detail::read_only_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::read_only() &&
{
    return flux::read_only(std::move(derived()));
}

} // namespace flux

#endif
