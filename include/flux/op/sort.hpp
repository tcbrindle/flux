
#ifndef FLUX_OP_SORT_HPP_INCLUDED
#define FLUX_OP_SORT_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/detail/pdqsort.hpp>

namespace flux {

namespace detail {

struct sort_fn {
    template <random_access_sequence Seq, typename Cmp = std::less<>,
              typename Proj = std::identity>
        requires bounded_sequence<Seq> &&
                 element_swappable_with<Seq, Seq>
    constexpr auto operator()(Seq&& seq, Cmp cmp = {}, Proj proj = {}) const
    {
        detail::pdqsort(seq, cmp, proj);
    }
};

} // namespace detail

inline constexpr auto sort = detail::sort_fn{};

} // namespace flux

#endif // FLUX_OP_SORT_HPP_INCLUDED
