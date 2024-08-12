
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_REQUIREMENTS_HPP_INCLUDED
#define FLUX_OP_REQUIREMENTS_HPP_INCLUDED

#include <flux/core/concepts.hpp>

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

} // namespace detail

namespace detail {

template <typename Func, typename E, distance_t N>
struct repeated_invocable_helper {
    template <std::size_t I>
    using repeater = E;

    static inline constexpr bool value = []<std::size_t... Is> (std::index_sequence<Is...>) consteval {
        return std::regular_invocable<Func, repeater<Is>...>;
    }(std::make_index_sequence<N>{});
};

template <typename Func, typename E, distance_t N>
concept repeated_invocable = repeated_invocable_helper<Func, E, N>::value;

template <typename InnerSeq, typename Pattern>
concept flatten_with_compatible =
    std::common_reference_with<element_t<InnerSeq>, element_t<Pattern>> &&
    std::common_reference_with<rvalue_element_t<InnerSeq>, rvalue_element_t<Pattern>> &&
    std::common_with<value_t<InnerSeq>, value_t<Pattern>>;

} // namespace detail

FLUX_EXPORT
template <typename Seq, typename Func, typename Init>
concept foldable =
    sequence<Seq> &&
    std::invocable<Func&, Init, element_t<Seq>> &&
    detail::foldable_<Seq, Func, Init>;

FLUX_EXPORT
template <typename Fn, typename Seq1, typename Seq2 = Seq1>
concept strict_weak_order_for =
    sequence<Seq1> &&
    sequence<Seq2> &&
    std::strict_weak_order<Fn&, element_t<Seq1>, element_t<Seq2>> &&
    std::strict_weak_order<Fn&, value_t<Seq1>&, element_t<Seq2>> &&
    std::strict_weak_order<Fn&, element_t<Seq1>, value_t<Seq2>&> &&
    std::strict_weak_order<Fn&, value_t<Seq1>&, value_t<Seq2>&> &&
    std::strict_weak_order<Fn&, common_element_t<Seq1>, common_element_t<Seq2>>;

} // namespace flux

#endif // FLUX_OP_REQUIREMENTS_HPP_INCLUDED
