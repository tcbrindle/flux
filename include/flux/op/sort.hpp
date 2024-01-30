
#ifndef FLUX_OP_SORT_HPP_INCLUDED
#define FLUX_OP_SORT_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/detail/pdqsort.hpp>
#include <flux/op/unchecked.hpp>

namespace flux {

namespace detail {

struct sort_fn {
    template <random_access_sequence Seq, typename Cmp = std::compare_three_way>
        requires bounded_sequence<Seq> &&
                 element_swappable_with<Seq, Seq> &&
                 strict_weak_order_for<Cmp, Seq>
    constexpr auto operator()(Seq&& seq, Cmp cmp = {}) const
    {
        auto wrapper = flux::unchecked(flux::from_fwd_ref(FLUX_FWD(seq)));
        auto comparator = [&cmp](auto&& lhs, auto&& rhs) {
            return std::is_lt(std::invoke(cmp, FLUX_FWD(lhs), FLUX_FWD(rhs)));
        };
        detail::pdqsort(wrapper, comparator);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto sort = detail::sort_fn{};

template <typename D>
template <typename Cmp>
    requires random_access_sequence<D> &&
             bounded_sequence<D> &&
             detail::element_swappable_with<D, D> &&
             strict_weak_order_for<Cmp, D>
constexpr void inline_sequence_base<D>::sort(Cmp cmp)
{
    return flux::sort(derived(), std::ref(cmp));
}

} // namespace flux

#endif // FLUX_OP_SORT_HPP_INCLUDED
