
// Copyright (c) 2024 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_FILTER_MAP_HPP_INCLUDED
#define FLUX_ADAPTOR_FILTER_MAP_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/adaptor/filter.hpp>
#include <flux/adaptor/map.hpp>

namespace flux {

namespace detail {

struct filter_map_fn {
    // If dereffing the optional would give us an rvalue reference,
    // prevent a probable dangling reference by returning by value instead
    template <typename T>
    using strip_rvalue_ref_t = std::conditional_t<
        std::is_rvalue_reference_v<T>, std::remove_reference_t<T>, T>;

    template <adaptable_iterable It, typename Func>
        requires(std::invocable<Func&, iterable_element_t<It>>
                 && optional_like<
                     std::remove_cvref_t<std::invoke_result_t<Func&, iterable_element_t<It>>>>)
    constexpr auto operator()(It&& it, Func func) const
    {
        // FIXME: Change this back to a pipeline later
        auto mapped = flux::map(FLUX_FWD(it), std::move(func));
        auto filtered
            = flux::filter(std::move(mapped), [](auto&& opt) { return static_cast<bool>(opt); });
        return flux::map(std::move(filtered),
                         [](auto&& opt) -> strip_rvalue_ref_t<decltype(*FLUX_FWD(opt))> {
                             return *FLUX_FWD(opt);
                         });
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto filter_map = detail::filter_map_fn{};

template <typename D>
template <typename Func>
requires std::invocable<Func&, element_t<D>> &&
         detail::optional_like<std::invoke_result_t<Func&, element_t<D>>>
constexpr auto inline_sequence_base<D>::filter_map(Func func) &&
{
    return flux::filter_map(derived(), std::move(func));
}

namespace detail
{

struct filter_deref_fn {
    template <adaptable_iterable It>
        requires optional_like<iterable_value_t<It>>
    constexpr auto operator()(It&& it) const
    {
        return filter_map(FLUX_FWD(it), [](auto&& opt) -> decltype(auto) { return FLUX_FWD(opt); });
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto filter_deref = detail::filter_deref_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::filter_deref() && requires detail::optional_like<value_t<D>>
{
    return flux::filter_deref(derived());
}
} // namespace flux

#endif // FLUX_ADAPTOR_FILTER_MAP_HPP_INCLUDED
