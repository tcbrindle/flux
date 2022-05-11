
#ifndef FLUX_OP_WRITE_TO_HPP_INCLUDED
#define FLUX_OP_WRITE_TO_HPP_INCLUDED

#include <flux/core/concepts.hpp>

#include <iosfwd>

namespace flux {

namespace write_to_detail {

struct fn {

    template <sequence Seq>
    constexpr auto operator()(Seq&& seq, std::ostream& os) const
        -> std::ostream&
    {
        os << '[';

        auto idx = flux::first(seq);
        if (!is_last(seq, idx)) {
            while (true) {
                if constexpr (sequence<element_t<Seq>>) {
                    (*this)(read_at(seq, idx), os);
                } else {
                    os << read_at(seq, idx);
                }

                inc(seq, idx);
                if (is_last(seq, idx)) {
                    break;
                }

                os << ", ";
            }
        }

        os << ']';
        return os;
    }
};

} // namespace write_to_detail

inline constexpr auto write_to = write_to_detail::fn{};

} // namespace flux

#endif // FLUX_OP_WRITE_TO_HPP_INCLUDED
