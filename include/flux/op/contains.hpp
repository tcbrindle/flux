
#ifndef FLUX_OP_CONTAINS_HPP_INCLUDED
#define FLUX_OP_CONTAINS_HPP_INCLUDED

#include <flux/op/for_each_while.hpp>

namespace flux {

namespace detail {

struct contains_fn {
    template <sequence Seq, typename Value, typename Proj = std::identity>
        requires std::equality_comparable_with<projected_t<Proj, Seq>, Value const&>
    constexpr auto operator()(Seq&& seq, Value const& value, Proj proj = {}) const
        -> bool
    {
        return !flux::is_last(seq, flux::for_each_while(seq, [&](auto&& elem) {
            return std::invoke(proj, FLUX_FWD(elem)) != value;
        }));
    }
};


} // namespace detail

inline constexpr auto contains = detail::contains_fn{};

template <typename D>
template <typename Value, typename Proj>
    requires std::equality_comparable_with<projected_t<Proj, D>, Value const&>
constexpr auto lens_base<D>::contains(Value const& value, Proj proj) -> bool
{
    return flux::contains(derived(), value, std::move(proj));
}

} // namespace flux

#endif
