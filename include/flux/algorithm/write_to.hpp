
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_WRITE_TO_HPP_INCLUDED
#define FLUX_ALGORITHM_WRITE_TO_HPP_INCLUDED

#include <flux/algorithm/for_each.hpp>

#include <iosfwd>

namespace flux {

namespace detail {

struct write_to_fn {

    template <iterable It, typename OStream>
        requires std::derived_from<OStream, std::ostream>
    auto operator()(It&& it, OStream& os) const
        -> std::ostream&
    {
        bool first = true;
        os << '[';

        flux::for_each(it, [&os, &first](auto&& elem) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }

            if constexpr (iterable<element_t<It>>) {
                write_to_fn{}(FLUX_FWD(elem), os);
            } else {
                os << FLUX_FWD(elem);
            }
        });

        os << ']';
        return os;
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto write_to = detail::write_to_fn{};

template <typename Derived>
auto inline_sequence_base<Derived>::write_to(std::ostream& os) -> std::ostream&
{
    return flux::write_to(derived(), os);
}

} // namespace flux

#endif // FLUX_ALGORITHM_WRITE_TO_HPP_INCLUDED
