
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_CONCEPTS_HPP_INCLUDED
#define FLUX_CORE_CONCEPTS_HPP_INCLUDED

#include <flux/core/macros.hpp>

#include <compare>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <type_traits>

namespace flux {

/*
 * Useful helpers
 */
template <typename From, typename To>
concept decays_to = std::same_as<std::remove_cvref_t<From>, To>;

namespace detail {

struct copy_fn {
    template <typename T>
    [[nodiscard]]
    constexpr auto operator()(T&& arg) const
        noexcept(std::is_nothrow_convertible_v<T, std::decay_t<T>>)
        -> std::decay_t<T>
    {
        return FLUX_FWD(arg);
    }
};

}

inline constexpr auto copy = detail::copy_fn{};

namespace detail {

template <std::integral To>
struct narrow_cast_fn {
    template <std::integral From>
    [[nodiscard]]
    constexpr auto operator()(From from) const -> To
    {
        if constexpr (requires { To{from}; }) {
            return To{from}; // not a narrowing conversion
        } else {
            To to = static_cast<To>(from);

            assert(static_cast<From>(to) == from &&
                   ((std::is_signed_v<From> == std::is_signed_v<To>) || ((to < To{}) == (from < From{}))));

            return to;
        }
    }
};

}

template <std::integral To>
inline constexpr auto narrow_cast = detail::narrow_cast_fn<To>{};

/*
 * Cursor concepts
 */
template <typename Cur>
concept cursor = std::movable<Cur>;

template <typename Cur>
concept regular_cursor = cursor<Cur> && std::regular<Cur>;

template <typename Cur>
concept ordered_cursor =
    regular_cursor<Cur> &&
    std::totally_ordered<Cur>;

/*
 * Sequence concepts and associated types
 */

template <typename T>
struct sequence_traits;

namespace detail {

template <typename T>
using traits_t = sequence_traits<std::remove_cvref_t<T>>;

} // namespace detail

template <typename Seq>
using cursor_t = decltype(detail::traits_t<Seq>::first(FLUX_DECLVAL(Seq&)));

template <typename Seq>
using element_t = decltype(detail::traits_t<Seq>::read_at(FLUX_DECLVAL(Seq&), FLUX_DECLVAL(cursor_t<Seq> const&)));

namespace detail {

template <typename T>
concept has_element_type = requires { typename element_t<T>; };

template <has_element_type T>
struct value_type { using type = std::remove_cvref_t<element_t<T>>; };

template <has_element_type T>
    requires requires { typename traits_t<T>::value_type; }
struct value_type<T> { using type = typename traits_t<T>::value_type; };

template <has_element_type T>
    requires requires { traits_t<T>::using_primary_template; } &&
             requires { typename T::value_type; }
struct value_type<T> { using type = typename T::value_type; };

template <has_element_type T>
struct rvalue_element_type {
    using type = std::conditional_t<std::is_lvalue_reference_v<element_t<T>>,
                                    std::add_rvalue_reference_t<std::remove_reference_t<element_t<T>>>,
                                    element_t<T>>;
};

template <typename Seq>
concept has_move_at = requires (Seq& seq, cursor_t<Seq> const& cur) {
   { traits_t<Seq>::move_at(seq, cur) };
};

template <has_element_type T>
    requires has_move_at<T>
struct rvalue_element_type<T> {
    using type = decltype(traits_t<T>::move_at(FLUX_DECLVAL(T&), FLUX_DECLVAL(cursor_t<T> const&)));
};

} // namespace detail

template <typename Seq>
using value_t = typename detail::value_type<Seq>::type;

#ifdef FLUX_DISTANCE_TYPE
using distance_t = FLUX_DISTANCE_TYPE;
static_assert(std::signed_integral<distance_t>,
              "Custom FLUX_DISTANCE_TYPE must be a signed integer type");
#else
using distance_t = std::ptrdiff_t;
#endif // FLUX_DISTANCE_TYPE

template <typename Seq>
using rvalue_element_t = typename detail::rvalue_element_type<Seq>::type;

template <typename Seq>
using common_element_t = std::common_reference_t<element_t<Seq>, value_t<Seq>&>;

namespace detail {

template <typename B>
concept boolean_testable =
    std::convertible_to<B, bool> &&
    requires (B&& b) {
        { !FLUX_FWD(b) } -> std::convertible_to<bool>;
    };

template <typename T>
using with_ref = T&;

template <typename T>
concept can_reference = requires { typename with_ref<T>; };

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept sequence_concept =
    requires (Seq& seq) {
        { Traits::first(seq) } -> cursor;
    } &&
    requires (Seq& seq, cursor_t<Seq> const& cur) {
        { Traits::is_last(seq, cur) } -> boolean_testable;
        { Traits::read_at(seq, cur) } -> can_reference;
    } &&
    requires (Seq& seq, cursor_t<Seq>& cur) {
        { Traits::inc(seq, cur) } -> std::same_as<cursor_t<Seq>&>;
    } &&
    std::common_reference_with<element_t<Seq>&&, rvalue_element_t<Seq>&&>;
    // FIXME FIXME: Need C++23 tuple changes, otherwise zip breaks these
/*    std::common_reference_with<element_t<Seq>&&, value_t<Seq>&> &&
    std::common_reference_with<rvalue_element_t<Seq>&&, value_t<Seq> const&>;*/

} // namespace detail

template <typename Seq>
concept sequence = detail::sequence_concept<Seq>;

namespace detail {

template <typename>
inline constexpr bool disable_multipass = false;

template <typename T>
    requires requires { T::disable_multipass; } &&
             decays_to<decltype(T::disable_multipass), bool>
inline constexpr bool disable_multipass<T> = T::disable_multipass;

} // namespace detail

template <typename Seq>
concept multipass_sequence =
    sequence<Seq> && regular_cursor<cursor_t<Seq>> &&
    !detail::disable_multipass<detail::traits_t<Seq>>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept bidirectional_sequence_concept =
    multipass_sequence<Seq> &&
    requires (Seq& seq, cursor_t<Seq>& cur) {
        { Traits::dec(seq, cur) } -> std::same_as<cursor_t<Seq>&>;
    };

} // namespace detail

template <typename Seq>
concept bidirectional_sequence = detail::bidirectional_sequence_concept<Seq>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept random_access_sequence_concept =
    bidirectional_sequence<Seq> && ordered_cursor<cursor_t<Seq>> &&
    requires (Seq& seq, cursor_t<Seq>& cur, distance_t offset) {
        { Traits::inc(seq, cur, offset) } -> std::same_as<cursor_t<Seq>&>;
    } &&
    requires (Seq& seq, cursor_t<Seq> const& cur) {
        { Traits::distance(seq, cur, cur) } -> std::convertible_to<distance_t>;
    };

} // namespace detail

template <typename Seq>
concept random_access_sequence = detail::random_access_sequence_concept<Seq>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept contiguous_sequence_concept =
    random_access_sequence<Seq> &&
    std::is_lvalue_reference_v<element_t<Seq>> &&
    std::same_as<value_t<Seq>, std::remove_cvref_t<element_t<Seq>>> &&
    requires (Seq& seq) {
        { Traits::data(seq) } -> std::same_as<std::add_pointer_t<element_t<Seq>>>;
    };

} // namespace detail

template <typename Seq>
concept contiguous_sequence = detail::contiguous_sequence_concept<Seq>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept bounded_sequence_concept =
    sequence<Seq> &&
    requires (Seq& seq) {
        { Traits::last(seq) } -> std::same_as<cursor_t<Seq>>;
    };

} // namespace detail

template <typename Seq>
concept bounded_sequence = detail::bounded_sequence_concept<Seq>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept sized_sequence_concept =
    sequence<Seq> &&
    (requires (Seq& seq) {
        { Traits::size(seq) } -> std::convertible_to<distance_t>;
    } || (
        random_access_sequence<Seq> && bounded_sequence<Seq>
    ));

} // namespace detail

template <typename Seq>
concept sized_sequence = detail::sized_sequence_concept<Seq>;

template <typename Seq, typename T>
concept writable_sequence_of =
    sequence<Seq> &&
    requires (element_t<Seq> elem, T&& item) {
        { elem = FLUX_FWD(item) } -> std::same_as<element_t<Seq>&>;
    };

namespace detail {

template <typename>
inline constexpr bool is_infinite_seq = false;

template <typename T>
    requires requires { T::is_infinite; } &&
                 decays_to<decltype(T::is_infinite), bool>
inline constexpr bool is_infinite_seq<T> = T::is_infinite;

}

template <typename Seq>
concept infinite_sequence =
    sequence<Seq> &&
    detail::is_infinite_seq<detail::traits_t<Seq>>;

namespace detail {

template <typename T, typename R = std::remove_cvref_t<T>>
constexpr bool is_ilist = false;

template <typename T, typename E>
constexpr bool is_ilist<T, std::initializer_list<E>> = true;

template <typename Seq>
concept rvalue_sequence =
    !std::is_reference_v<Seq> &&
    std::movable<Seq> &&
    sequence<Seq>;

template <typename Seq>
concept trivially_copyable_sequence =
    std::copyable<Seq> &&
    std::is_trivially_copyable_v<Seq> &&
    sequence<Seq>;

}

template <typename Seq>
concept adaptable_sequence =
    (detail::rvalue_sequence<Seq>
         || (std::is_lvalue_reference_v<Seq> &&
             detail::trivially_copyable_sequence<std::decay_t<Seq>>)) &&
    !detail::is_ilist<Seq>;

template <typename D>
struct lens_base;

namespace detail {

template <typename T, typename U>
    requires (!std::same_as<T, lens_base<U>>)
void derived_from_lens_base_test(T const&, lens_base<U> const&);

template <typename T>
concept derived_from_lens_base = requires(T t) {
    derived_from_lens_base_test(t, t);
};

} // namespace detail

template <typename Seq>
concept lens =
    sequence<Seq> &&
    std::movable<Seq> &&
    detail::derived_from_lens_base<Seq>;

/*
 * More useful concepts and associated types
 */
template <typename Proj, sequence Seq>
using projected_t = std::invoke_result_t<Proj&, element_t<Seq>>;

template <typename Pred, typename Seq, typename Proj>
concept predicate_for =
    std::predicate<Pred&, std::invoke_result_t<Proj&, element_t<Seq>>>;

/*
 * Default sequence_traits implementation
 */
namespace detail {

template <typename T>
concept has_nested_sequence_traits =
    requires { typename T::flux_sequence_traits; } &&
    std::is_class_v<typename T::flux_sequence_traits>;

}

template <typename T>
    requires detail::has_nested_sequence_traits<T>
struct sequence_traits<T> : T::flux_sequence_traits {};


} // namespace flux

#endif // FLUX_CORE_CONCEPTS_HPP_INCLUDED
