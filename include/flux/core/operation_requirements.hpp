
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_OPERATION_REQUIREMENTS_HPP_INCLUDED
#define FLUX_CORE_OPERATION_REQUIREMENTS_HPP_INCLUDED

#include <flux/core/concepts.hpp>
#include <flux/core/iterable_concepts.hpp>

namespace flux {

FLUX_EXPORT
template <typename Seq, typename Func, typename Init>
using fold_result_t = std::decay_t<std::invoke_result_t<Func&, Init, element_t<Seq>>>;

namespace detail {

template <typename Seq, typename Func, typename Init,
          typename R = fold_result_t<Seq, Func, Init>>
concept foldable_ =
    std::invocable<Func&, R, element_t<Seq>> &&
    std::convertible_to<Init, R> &&
    std::assignable_from<R&, std::invoke_result_t<Func&, R, element_t<Seq>>>;

template <typename Func, typename E, int_t N>
struct repeated_invocable_helper {
    template <std::size_t I>
    using repeater = E;

    static inline constexpr bool value = []<std::size_t... Is> (std::index_sequence<Is...>) consteval {
        return std::regular_invocable<Func, repeater<Is>...>;
    }(std::make_index_sequence<N>{});
};

template <typename Func, typename E, int_t N>
concept repeated_invocable = repeated_invocable_helper<Func, E, N>::value;

template <typename InnerSeq, typename Pattern>
concept flatten_with_compatible
    = std::common_reference_with<iterable_element_t<InnerSeq>, iterable_element_t<Pattern>>
    && std::common_with<iterable_value_t<InnerSeq>, iterable_value_t<Pattern>>;

} // namespace detail

FLUX_EXPORT
template <typename Seq, typename Func, typename Init>
concept foldable =
    sequence<Seq> &&
    std::invocable<Func&, Init, element_t<Seq>> &&
    detail::foldable_<Seq, Func, Init>;

FLUX_EXPORT
template <typename Fn, typename It1, typename It2 = It1>
concept weak_ordering_for = iterable<It1> && iterable<It2>
    && ordering_invocable<Fn&, iterable_element_t<It1>, iterable_element_t<It2>, std::weak_ordering>
    && ordering_invocable<Fn&, iterable_value_t<It1>&, iterable_value_t<It2>&, std::weak_ordering>
    && ordering_invocable<Fn&, iterable_value_t<It1>&, iterable_value_t<It2>&, std::weak_ordering>
    && ordering_invocable<Fn&, iterable_common_element_t<It1>, iterable_common_element_t<It2>,
                          std::weak_ordering>;

} // namespace flux

#endif // FLUX_CORE_OPERATION_REQUIREMENTS_HPP_INCLUDED
