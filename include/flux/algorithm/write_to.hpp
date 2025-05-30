
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_WRITE_TO_HPP_INCLUDED
#define FLUX_ALGORITHM_WRITE_TO_HPP_INCLUDED

#include <flux/algorithm/for_each.hpp>

#include <iosfwd>

namespace flux {

FLUX_EXPORT
struct write_to_t {
    template <iterable It, typename OStream>
        requires std::derived_from<OStream, std::ostream>
    auto operator()(It&& it, OStream& os) const -> OStream&
    {
        bool first = true;
        os << '[';

        for_each(it, [&os, &first](auto&& elem) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }

            if constexpr (iterable<iterable_element_t<It>>) {
                write_to_t{}(FLUX_FWD(elem), os);
            } else {
                os << FLUX_FWD(elem);
            }
        });

        os << ']';
        return os;
    }
};

FLUX_EXPORT inline constexpr write_to_t write_to{};

template <typename Derived>
auto inline_sequence_base<Derived>::write_to(std::ostream& os) -> std::ostream&
{
    return flux::write_to(derived(), os);
}

} // namespace flux

#endif // FLUX_ALGORITHM_WRITE_TO_HPP_INCLUDED
