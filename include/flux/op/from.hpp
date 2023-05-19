
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FROM_HPP_INCLUDED
#define FLUX_OP_FROM_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/ref.hpp>

namespace flux {

namespace detail {

struct from_fn {
    template <sequence Seq>
        requires (std::is_lvalue_reference_v<Seq> || adaptable_sequence<Seq>)
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr (std::is_lvalue_reference_v<Seq>) {
            if constexpr (std::is_const_v<std::remove_reference_t<Seq>>) {
                return flux::ref(seq);
            } else {
                return flux::mut_ref(seq);
            }
        } else {
            return detail::owning_adaptor<Seq>(FLUX_FWD(seq));
        }
    }
};

} // namespace detail

inline constexpr auto from = detail::from_fn{};

} // namespace flux

#endif // FLUX_OP_FROM_HPP_INCLUDED
