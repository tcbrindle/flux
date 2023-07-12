
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_READ_ONLY_HPP_INCLUDED
#define FLUX_OP_READ_ONLY_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/map.hpp>

namespace flux {

namespace detail {

template <typename T>
struct cast_to_const {
    constexpr auto operator()(auto&& elem) const -> T { return FLUX_FWD(elem); }
};

template <sequence Base>
    requires (not read_only_sequence<Base>)
struct read_only_adaptor : map_adaptor<Base, cast_to_const<const_element_t<Base>>> {
private:
    using map = map_adaptor<Base, cast_to_const<const_element_t<Base>>>;

public:
    constexpr explicit read_only_adaptor(decays_to<Base> auto&& base)
        : map(FLUX_FWD(base), cast_to_const<const_element_t<Base>>{})
    {}

    struct flux_sequence_traits : map::flux_sequence_traits {
        using value_type = value_t<Base>;

        static constexpr auto data(auto& self)
            requires contiguous_sequence<Base>
        {
            using P = std::add_pointer_t<std::remove_reference_t<const_element_t<Base>>>;
            return static_cast<P>(flux::data(self.base()));
        }
    };
};

struct read_only_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> read_only_sequence auto
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
