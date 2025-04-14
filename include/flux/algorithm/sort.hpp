
#ifndef FLUX_ALGORITHM_SORT_HPP_INCLUDED
#define FLUX_ALGORITHM_SORT_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/algorithm/detail/pdqsort.hpp>
#include <flux/adaptor/unchecked.hpp>

namespace flux {

namespace detail {

struct sort_fn {
    template <random_access_sequence Seq, typename Cmp = std::compare_three_way>
        requires bounded_sequence<Seq> &&
                 element_swappable_with<Seq, Seq> &&
                 weak_ordering_for<Cmp, Seq>
    constexpr auto operator()(Seq&& seq, Cmp cmp = {}) const
    {
        auto wrapper = flux::unchecked(flux::from_fwd_ref(FLUX_FWD(seq)));
        detail::pdqsort(wrapper, cmp);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto sort = detail::sort_fn{};

template <typename D>
template <typename Cmp>
    requires random_access_sequence<D> &&
             bounded_sequence<D> &&
             detail::element_swappable_with<D, D> &&
             weak_ordering_for<Cmp, D>
constexpr void inline_iter_base<D>::sort(Cmp cmp)
{
    return flux::sort(derived(), std::ref(cmp));
}

} // namespace flux

#endif // FLUX_ALGORITHM_SORT_HPP_INCLUDED
