
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_CONCEPTS_HPP_INCLUDED
#define FLUX_CORE_CONCEPTS_HPP_INCLUDED

#include <flux/core/utils.hpp>

#include <compare>
#include <concepts>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <type_traits>

// clang-format off

// Workaround GCC12 ICE in sequence concept definition below
#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ < 13)
#define FLUX_COMPILER_IS_GCC12
#endif

#if defined(__cpp_lib_ranges_zip) && (__cpp_lib_ranges_zip >= 202110L)
#define FLUX_HAVE_CPP23_TUPLE_COMMON_REF
#endif

namespace flux {

/*
 * Cursor concepts
 */
FLUX_EXPORT
template <typename Cur>
concept cursor = std::movable<Cur>;

FLUX_EXPORT
template <typename Cur>
concept regular_cursor = cursor<Cur> && std::regular<Cur>;

FLUX_EXPORT
template <typename Cur>
concept ordered_cursor =
    regular_cursor<Cur> &&
    std::totally_ordered<Cur>;

/*
 * Sequence concepts and associated types
 */

FLUX_EXPORT
template <typename T>
struct sequence_traits;

FLUX_EXPORT
struct default_sequence_traits;

namespace detail {

template <typename T>
using traits_t = sequence_traits<std::remove_cvref_t<T>>;

} // namespace detail

FLUX_EXPORT
template <typename Seq>
using cursor_t = decltype(detail::traits_t<Seq>::first(FLUX_DECLVAL(Seq&)));

FLUX_EXPORT
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

} // namespace detail

FLUX_EXPORT
template <typename Seq>
using value_t = typename detail::value_type<Seq>::type;

FLUX_EXPORT
using distance_t = flux::config::int_type;

FLUX_EXPORT
using index_t = flux::config::int_type;

FLUX_EXPORT
template <typename Seq>
using rvalue_element_t = decltype(detail::traits_t<Seq>::move_at(FLUX_DECLVAL(Seq&), FLUX_DECLVAL(cursor_t<Seq> const&)));

FLUX_EXPORT
template <typename Seq>
using common_element_t = std::common_reference_t<element_t<Seq>, value_t<Seq>&>;

FLUX_EXPORT
template <typename Seq>
using const_element_t = std::common_reference_t<value_t<Seq> const&&, element_t<Seq>>;

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
concept sequence_requirements =
    requires (Seq& seq) {
        { Traits::first(seq) } -> cursor;
    } &&
    requires (Seq& seq, cursor_t<Seq> const& cur) {
        { Traits::is_last(seq, cur) } -> boolean_testable;
        { Traits::read_at(seq, cur) } -> can_reference;
    } &&
    requires (Seq& seq, cursor_t<Seq>& cur) {
        { Traits::inc(seq, cur) };
    };

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept sequence_concept =
    sequence_requirements<Seq> &&
    requires (Seq& seq, cursor_t<Seq> const& cur) {
        { Traits::read_at_unchecked(seq, cur) } -> std::same_as<element_t<Seq>>;
        { Traits::move_at(seq, cur) } -> can_reference;
        { Traits::move_at_unchecked(seq, cur) } -> std::same_as<rvalue_element_t<Seq>>;
    } &&
#ifndef FLUX_COMPILER_IS_GCC12
    requires (Seq& seq, bool (*pred)(element_t<Seq>)) {
        { Traits::for_each_while(seq, pred) } -> std::same_as<cursor_t<Seq>>;
    } &&
#endif
#ifdef FLUX_HAVE_CPP23_TUPLE_COMMON_REF
    std::common_reference_with<element_t<Seq>&&, value_t<Seq>&> &&
    std::common_reference_with<rvalue_element_t<Seq>&&, value_t<Seq> const&> &&
#endif
    std::common_reference_with<element_t<Seq>&&, rvalue_element_t<Seq>&&>;

} // namespace detail

FLUX_EXPORT
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

FLUX_EXPORT
template <typename Seq>
concept multipass_sequence =
    sequence<Seq> && regular_cursor<cursor_t<Seq>> &&
    !detail::disable_multipass<detail::traits_t<Seq>>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept bidirectional_sequence_requirements =
    requires (Seq& seq, cursor_t<Seq>& cur) {
        { Traits::dec(seq, cur) };
    };

} // namespace detail

FLUX_EXPORT
template <typename Seq>
concept bidirectional_sequence = multipass_sequence<Seq> && detail::bidirectional_sequence_requirements<Seq>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept random_access_sequence_requirements =
    ordered_cursor<cursor_t<Seq>> &&
    requires (Seq& seq, cursor_t<Seq>& cur, distance_t offset) {
        { Traits::inc(seq, cur, offset) };
    } &&
    requires (Seq& seq, cursor_t<Seq> const& cur) {
        { Traits::distance(seq, cur, cur) } -> std::convertible_to<distance_t>;
    };

} // namespace detail

FLUX_EXPORT
template <typename Seq>
concept random_access_sequence =
    bidirectional_sequence<Seq> &&
    detail::random_access_sequence_requirements<Seq>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept bounded_sequence_requirements =
    requires (Seq& seq) {
        { Traits::last(seq) } -> std::same_as<cursor_t<Seq>>;
    };

} // namespace detail

FLUX_EXPORT
template <typename Seq>
concept bounded_sequence = sequence<Seq> && detail::bounded_sequence_requirements<Seq>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept contiguous_sequence_requirements =
    std::is_lvalue_reference_v<element_t<Seq>> &&
    std::same_as<value_t<Seq>, std::remove_cvref_t<element_t<Seq>>> &&
    requires (Seq& seq) {
        { Traits::data(seq) } -> std::same_as<std::add_pointer_t<element_t<Seq>>>;
    };

} // namespace detail

FLUX_EXPORT
template <typename Seq>
concept contiguous_sequence =
    random_access_sequence<Seq> &&
    bounded_sequence<Seq> &&
    detail::contiguous_sequence_requirements<Seq>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept sized_sequence_requirements =
    requires (Seq& seq) {
        { Traits::size(seq) } -> std::convertible_to<distance_t>;
    };

} // namespace detail

FLUX_EXPORT
template <typename Seq>
concept sized_sequence = sequence<Seq> && detail::sized_sequence_requirements<Seq>;

FLUX_EXPORT
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

FLUX_EXPORT
template <typename Seq>
concept infinite_sequence =
    sequence<Seq> &&
    detail::is_infinite_seq<detail::traits_t<Seq>>;

FLUX_EXPORT
template <typename Seq>
concept read_only_sequence =
    sequence<Seq> &&
    std::same_as<element_t<Seq>, const_element_t<Seq>>;

FLUX_EXPORT
template <typename Seq>
concept const_iterable_sequence =
    // Seq and Seq const must both be sequences
    sequence<Seq> && sequence<Seq const> &&
    // Seq and Seq const must have the same cursor and value types
    std::same_as<cursor_t<Seq>, cursor_t<Seq const>> &&
    std::same_as<value_t<Seq>, value_t<Seq const>> &&
    // Seq and Seq const must have the same const_element type
#ifdef FLUX_HAVE_CPP23_TUPLE_COMMON_REF
    std::same_as<const_element_t<Seq>, const_element_t<Seq const>> &&
#endif
    // Seq and Seq const must model the same extended sequence concepts
    (multipass_sequence<Seq> == multipass_sequence<Seq const>) &&
    (bidirectional_sequence<Seq> == bidirectional_sequence<Seq const>) &&
    (random_access_sequence<Seq> == random_access_sequence<Seq const>) &&
    (contiguous_sequence<Seq> == contiguous_sequence<Seq const>) &&
    (bounded_sequence<Seq> == bounded_sequence<Seq const>) &&
    (sized_sequence<Seq> == sized_sequence<Seq const>) &&
    (infinite_sequence<Seq> == infinite_sequence<Seq const>) &&
    // If Seq is read-only, Seq const must be read-only as well
    (!read_only_sequence<Seq> || read_only_sequence<Seq const>);

namespace detail {

template <typename T, typename R = std::remove_cvref_t<T>>
constexpr bool is_ilist = false;

template <typename T, typename E>
constexpr bool is_ilist<T, std::initializer_list<E>> = true;

template <typename Seq>
concept rvalue_sequence =
    std::is_object_v<Seq> &&
    std::move_constructible<Seq> &&
    sequence<Seq>;

template <typename Seq>
concept trivially_copyable_sequence =
    std::copyable<Seq> &&
    std::is_trivially_copyable_v<Seq> &&
    sequence<Seq>;

}

FLUX_EXPORT
template <typename Seq>
concept adaptable_sequence =
    (detail::rvalue_sequence<Seq>
         || (std::is_lvalue_reference_v<Seq> &&
             detail::trivially_copyable_sequence<std::decay_t<Seq>>)) &&
    !detail::is_ilist<Seq>;

FLUX_EXPORT
template <typename D>
struct inline_sequence_base;

namespace detail {

template <typename T, typename U>
    requires (!std::same_as<T, inline_sequence_base<U>>)
void derived_from_inline_sequence_base_test(T const&, inline_sequence_base<U> const&);

template <typename T>
concept derived_from_inline_sequence_base = requires(T t) {
    derived_from_inline_sequence_base_test(t, t);
};

} // namespace detail


/*
 * Default sequence_traits implementation
 */

struct default_sequence_traits {

    template <typename Self>
        requires detail::sequence_requirements<Self>
    static constexpr auto read_at_unchecked(Self& self, cursor_t<Self> const& cur)
        -> decltype(detail::traits_t<Self>::read_at(self, cur))
    {
        return detail::traits_t<Self>::read_at(self, cur);
    }

    template <typename Self>
        requires detail::sequence_requirements<Self>
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
        -> std::conditional_t<std::is_lvalue_reference_v<element_t<Self>>,
                              std::add_rvalue_reference_t<std::remove_reference_t<element_t<Self>>>,
                              element_t<Self>>
    {
        using Traits = detail::traits_t<Self>;
        if constexpr (std::is_lvalue_reference_v<element_t<Self>>) {
            return std::move(Traits::read_at(self, cur));
        } else {
            return Traits::read_at(self, cur);
        }
    }

    template <typename Self>
        requires detail::sequence_requirements<Self>
    static constexpr auto move_at_unchecked(Self& self, cursor_t<Self> const& cur)
        -> decltype(detail::traits_t<Self>::move_at(self, cur))
    {
        return detail::traits_t<Self>::move_at(self, cur);
    }

    template <typename Self>
        requires detail::random_access_sequence_requirements<Self> &&
                 detail::bounded_sequence_requirements<Self>
    static constexpr auto size(Self& self) -> distance_t
    {
        using Traits = detail::traits_t<Self>;
        return Traits::distance(self, Traits::first(self), Traits::last(self));
    }

    template <typename Self, typename Pred>
        requires detail::sequence_requirements<Self>
    static constexpr auto for_each_while(Self& self, Pred&& pred) -> cursor_t<Self>
    {
        using Traits = detail::traits_t<Self>;

        auto cur = Traits::first(self);
        if constexpr (bounded_sequence<Self> && regular_cursor<cursor_t<Self>>) {
            auto const last = Traits::last(self);
            while (cur != last) {
                if (!std::invoke(pred, Traits::read_at(self, cur))) {
                    break;
                }
                Traits::inc(self, cur);
            }
        } else {
            while (!Traits::is_last(self, cur)) {
                if (!std::invoke(pred, Traits::read_at(self, cur))) {
                    break;
                }
                Traits::inc(self, cur);
            }
        }
        return cur;
    }

};

namespace detail {

template <typename T>
concept has_nested_sequence_traits =
    requires { typename T::flux_sequence_traits; } &&
    std::is_class_v<typename T::flux_sequence_traits>;

}

template <typename T>
    requires detail::has_nested_sequence_traits<T>
struct sequence_traits<T> : T::flux_sequence_traits {};

namespace detail {

template <typename O>
concept optional_like =
    std::default_initializable<O> &&
    std::movable<O> &&
    requires (O& o) {
        { static_cast<bool>(o) };
        { *o } -> flux::detail::can_reference;
    };

}

} // namespace flux

#endif // FLUX_CORE_CONCEPTS_HPP_INCLUDED
