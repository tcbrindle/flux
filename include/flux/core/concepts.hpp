
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
 * Useful helper concepts
 */
template <typename From, typename To>
concept decays_to = std::same_as<std::remove_cvref_t<From>, To>;

/*
 * Index concepts
 */
template <typename Idx>
concept index = std::movable<Idx>;

template <typename Idx>
concept regular_index = index<Idx> && std::regular<Idx>;

template <typename Idx>
concept ordered_index =
    regular_index<Idx> &&
    std::totally_ordered<Idx>;

/*
 * Sequence concepts and associated types
 */

template <typename T>
struct sequence_iface;

namespace detail {

template <typename T>
using iface_t = sequence_iface<std::remove_cvref_t<T>>;

} // namespace detail

template <typename Seq>
using index_t = decltype(detail::iface_t<Seq>::first(FLUX_DECLVAL(Seq&)));

template <typename Seq>
using element_t = decltype(detail::iface_t<Seq>::read_at(FLUX_DECLVAL(Seq&), FLUX_DECLVAL(index_t<Seq> const&)));

namespace detail {

template <typename T>
concept has_element_type = requires { typename element_t<T>; };

template <has_element_type T>
struct value_type { using type = std::remove_cvref_t<element_t<T>>; };

template <has_element_type T>
    requires requires { typename iface_t<T>::value_type; }
struct value_type<T> { using type = typename iface_t<T>::value_type; };

template <has_element_type T>
    requires requires { iface_t<T>::using_primary_template; } &&
             requires { typename T::value_type; }
struct value_type<T> { using type = typename T::value_type; };

template <typename>
struct distance_type { using type = std::ptrdiff_t; };

template <typename T>
    requires requires { typename iface_t<T>::distance_type; }
struct distance_type<T> { using type = typename iface_t<T>::distance_type; };

template <typename T>
    requires requires { iface_t<T>::using_primary_template; } &&
             requires { typename std::decay_t<T>::distance_type; }
struct distance_type<T> { using type = typename std::decay_t<T>::distance_type; };

template <has_element_type T>
struct rvalue_element_type {
    using type = std::conditional_t<std::is_lvalue_reference_v<element_t<T>>,
                                    std::add_rvalue_reference_t<std::remove_reference_t<element_t<T>>>,
                                    element_t<T>>;
};

template <typename Seq>
concept has_move_at = requires (Seq& seq, index_t<Seq> const& idx) {
   { iface_t<Seq>::move_at(seq, idx) };
};

template <has_element_type T>
    requires has_move_at<T>
struct rvalue_element_type<T> {
    using type = decltype(iface_t<T>::move_at(FLUX_DECLVAL(T&), FLUX_DECLVAL(index_t<T> const&)));
};

} // namespace detail

template <typename Seq>
using value_t = typename detail::value_type<Seq>::type;

template <typename Seq>
using distance_t = typename detail::distance_type<Seq>::type;

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

template <typename Seq, typename Iface = sequence_iface<std::remove_cvref_t<Seq>>>
concept sequence_impl =
    requires (Seq& seq) {
        { Iface::first(seq) } -> index;
    } &&
    requires (Seq& seq, index_t<Seq> const& idx) {
        { Iface::is_last(seq, idx) } -> boolean_testable;
        { Iface::read_at(seq, idx) } -> can_reference;
    } &&
    requires (Seq& seq, index_t<Seq>& idx) {
        { Iface::inc(seq, idx) } -> std::same_as<index_t<Seq>&>;
    } &&
    std::signed_integral<distance_t<Seq>> &&
    std::common_reference_with<element_t<Seq>&&, rvalue_element_t<Seq>&&>;
    // FIXME FIXME: Need C++23 tuple changes, otherwise zip breaks these
/*    std::common_reference_with<element_t<Seq>&&, value_t<Seq>&> &&
    std::common_reference_with<rvalue_element_t<Seq>&&, value_t<Seq> const&>;*/

} // namespace detail

template <typename Seq>
concept sequence = detail::sequence_impl<Seq>;

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
    sequence<Seq> && regular_index<index_t<Seq>> &&
    !detail::disable_multipass<detail::iface_t<Seq>>;

namespace detail {

template <typename Seq, typename Iface = sequence_iface<std::remove_cvref_t<Seq>>>
concept bidirectional_sequence_impl =
    multipass_sequence<Seq> &&
    requires (Seq& seq, index_t<Seq>& idx) {
        { Iface::dec(seq, idx) } -> std::same_as<index_t<Seq>&>;
    };

} // namespace detail

template <typename Seq>
concept bidirectional_sequence = detail::bidirectional_sequence_impl<Seq>;

namespace detail {

template <typename Seq, typename Iface = sequence_iface<std::remove_cvref_t<Seq>>>
concept random_access_sequence_impl =
    bidirectional_sequence<Seq> && ordered_index<index_t<Seq>> &&
    requires (Seq& seq, index_t<Seq>& idx, distance_t<Seq> offset) {
        { Iface::inc(seq, idx, offset) } -> std::same_as<index_t<Seq>&>;
    } &&
    requires (Seq& seq, index_t<Seq> const& idx) {
        { Iface::distance(seq, idx, idx) } -> std::convertible_to<distance_t<Seq>>;
    };

} // namespace detail

template <typename Seq>
concept random_access_sequence = detail::random_access_sequence_impl<Seq>;

namespace detail {

template <typename Seq, typename Iface = sequence_iface<std::remove_cvref_t<Seq>>>
concept contiguous_sequence_impl =
    random_access_sequence<Seq> &&
    std::is_lvalue_reference_v<element_t<Seq>> &&
    std::same_as<value_t<Seq>, std::remove_cvref_t<element_t<Seq>>> &&
    requires (Seq& seq) {
        { Iface::data(seq) } -> std::same_as<std::add_pointer_t<element_t<Seq>>>;
    };

} // namespace detail

template <typename Seq>
concept contiguous_sequence = detail::contiguous_sequence_impl<Seq>;

namespace detail {

template <typename Seq, typename Iface = sequence_iface<std::remove_cvref_t<Seq>>>
concept bounded_sequence_impl =
    sequence<Seq> &&
    requires (Seq& seq) {
        { Iface::last(seq) } -> std::same_as<index_t<Seq>>;
    };

} // namespace detail

template <typename Seq>
concept bounded_sequence = detail::bounded_sequence_impl<Seq>;

namespace detail {

template <typename Seq, typename Iface = sequence_iface<std::remove_cvref_t<Seq>>>
concept sized_sequence_impl =
    sequence<Seq> &&
    (requires (Seq& seq) {
        { Iface::size(seq) } -> std::convertible_to<distance_t<Seq>>;
    } || (
        random_access_sequence<Seq> && bounded_sequence<Seq>
    ));

} // namespace detail

template <typename Seq>
concept sized_sequence = detail::sized_sequence_impl<Seq>;

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
    detail::is_infinite_seq<detail::iface_t<Seq>>;


template <typename D>
    requires std::is_class_v<D> && std::same_as<D, std::remove_cv_t<D>>
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
 * Default sequence_iface implementation
 */
template <typename T>
struct sequence_iface {

    static_assert(!detail::derived_from_lens_base<T>,
        "Types deriving from lens_base cannot define the sequence interface "
        "in-class. Provide a separate specialisation of sequence_iface instead.");

    static constexpr bool using_primary_template = true;

    static constexpr bool disable_multipass = detail::disable_multipass<T>;
    static constexpr bool is_infinite = detail::is_infinite_seq<T>;

    /* sequence interface */
    static constexpr auto first(decays_to<T> auto& self)
        -> decltype(self.first())
    {
        return self.first();
    }

    template <decays_to<T> Self>
    static constexpr auto is_last(Self& self, index_t<Self> const& idx)
        -> decltype(self.is_last(idx))
    {
        return self.is_last(idx);
    }

    template <decays_to<T> Self>
    static constexpr auto read_at(Self& self, index_t<Self> const& idx)
        -> decltype(self.read_at(idx))
    {
        return self.read_at(idx);
    }

    template <decays_to<T> Self>
    static constexpr auto inc(Self& self, index_t<Self>& idx)
        -> decltype(self.inc(idx))
    {
        return self.inc(idx);
    }

    /* bidirectional sequence interface */
    template <decays_to<T> Self>
    static constexpr auto dec(Self& self, index_t<Self>& idx)
        -> decltype(self.dec(idx))
    {
        return self.dec(idx);
    }

    /* random-access sequence interface */
    template <decays_to<T> Self>
    static constexpr auto inc(Self& self, index_t<Self>& idx, distance_t<Self> offset)
        -> decltype(self.inc(idx, offset))
    {
        return self.inc(idx, offset);
    }

    template <decays_to<T> Self>
    static constexpr auto distance(Self& self, index_t<Self> const& from, index_t<Self> const& to)
        -> decltype(self.distance(from, to))
    {
        return self.distance(from, to);
    }

    /* contiguous sequence interface */
    static constexpr auto data(decays_to<T> auto& self)
        -> decltype(self.data())
    {
        return self.data();
    }

    /* sized sequence interface */
    static constexpr auto size(decays_to<T> auto& self)
        -> decltype(self.size())
    {
        return self.size();
    }

    /* bounded sequence interface */
    static constexpr auto last(decays_to<T> auto& self)
        -> decltype(self.last())
    {
        return self.last();
    }

    /* other customisation points */
    template <decays_to<T> Self>
    static constexpr auto move_at(Self& self, index_t<Self> const& idx)
        -> decltype(self.move_at(idx))
    {
        return self.move_at(idx);
    }

    template <decays_to<T> Self, typename Pred>
    static constexpr auto for_each_while(Self& self, Pred&& pred)
        -> decltype(self.for_each_while(FLUX_FWD(pred)))
    {
        return self.for_each_while(FLUX_FWD(pred));
    }
};

} // namespace flux

#endif // FLUX_CORE_CONCEPTS_HPP_INCLUDED
