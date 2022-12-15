
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
                 element_swappable_with<Seq, Seq> &&
                 std::predicate<Cmp&, projected_t<Proj, Seq>, projected_t<Proj, Seq>>
    constexpr auto operator()(Seq&& seq, Cmp cmp = {}, Proj proj = {}) const
    {
        detail::pdqsort(seq, cmp, proj);
    }
};

} // namespace detail

inline constexpr auto sort = detail::sort_fn{};

template <typename D>
template <typename Cmp, typename Proj>
    requires random_access_sequence<D> &&
             bounded_sequence<D> &&
             detail::element_swappable_with<D, D> &&
             std::predicate<Cmp&, projected_t<Proj, D>, projected_t<Proj, D>>
constexpr void inline_sequence_base<D>::sort(Cmp cmp, Proj proj)
{
    return flux::sort(derived(), std::move(cmp), std::move(proj));
}

} // namespace flux

#endif // FLUX_OP_SORT_HPP_INCLUDED
