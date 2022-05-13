
#ifndef FLUX_OP_WRITE_TO_HPP_INCLUDED
#define FLUX_OP_WRITE_TO_HPP_INCLUDED

#include <flux/op/for_each.hpp>

#include <iosfwd>

namespace flux {

namespace write_to_detail {

struct fn {

    template <sequence Seq>
    auto operator()(Seq&& seq, std::ostream& os) const
        -> std::ostream&
    {
        bool first = true;
        os << '[';

        flux::for_each(FLUX_FWD(seq), [&os, &first](auto&& elem) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }

            if constexpr (sequence<element_t<Seq>>) {
                fn{}(FLUX_FWD(elem), os);
            } else {
                os << FLUX_FWD(elem);
            }
        });

        os << ']';
        return os;
    }
};

} // namespace write_to_detail

inline constexpr auto write_to = write_to_detail::fn{};

template <typename Derived>
auto lens_base<Derived>::write_to(std::ostream& os) -> std::ostream&
{
    return flux::write_to(derived(), os);
}

} // namespace flux

#endif // FLUX_OP_WRITE_TO_HPP_INCLUDED
