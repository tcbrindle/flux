
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_HPP_INCLUDED
#define FLUX_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_HPP_INCLUDED
#define FLUX_CORE_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_CONCEPTS_HPP_INCLUDED
#define FLUX_CORE_CONCEPTS_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_MACROS_HPP_INCLUDED
#define FLUX_CORE_MACROS_HPP_INCLUDED

#include <version>

#define FLUX_FWD(x) static_cast<decltype(x)&&>(x)

#define FLUX_DECLVAL(...)  ((static_cast<__VA_ARGS__(*)()noexcept>(nullptr))())

#ifdef __GNUC__
#define FLUX_ALWAYS_INLINE [[gnu::always_inline]]
#else
#define FLUX_ALWAYS_INLINE
#endif

#define FLUX_NO_UNIQUE_ADDRESS [[no_unique_address]]

#define FLUX_FOR(_flux_var_decl_, ...)                    \
    if (auto&& _flux_seq_ = __VA_ARGS__; true)           \
        for (auto _flux_cur_ = ::flux::first(_flux_seq_);   \
             !::flux::is_last(_flux_seq_, _flux_cur_);     \
              ::flux::inc(_flux_seq_, _flux_cur_))         \
            if (_flux_var_decl_ = ::flux::read_at(_flux_seq_, _flux_cur_); true)

#endif // FLUX_CORE_MACROS_HPP_INCLUDED


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
struct sequence_iface;

namespace detail {

template <typename T>
using iface_t = sequence_iface<std::remove_cvref_t<T>>;

} // namespace detail

template <typename Seq>
using cursor_t = decltype(detail::iface_t<Seq>::first(FLUX_DECLVAL(Seq&)));

template <typename Seq>
using element_t = decltype(detail::iface_t<Seq>::read_at(FLUX_DECLVAL(Seq&), FLUX_DECLVAL(cursor_t<Seq> const&)));

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
concept has_move_at = requires (Seq& seq, cursor_t<Seq> const& cur) {
   { iface_t<Seq>::move_at(seq, cur) };
};

template <has_element_type T>
    requires has_move_at<T>
struct rvalue_element_type<T> {
    using type = decltype(iface_t<T>::move_at(FLUX_DECLVAL(T&), FLUX_DECLVAL(cursor_t<T> const&)));
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
        { Iface::first(seq) } -> cursor;
    } &&
    requires (Seq& seq, cursor_t<Seq> const& cur) {
        { Iface::is_last(seq, cur) } -> boolean_testable;
        { Iface::read_at(seq, cur) } -> can_reference;
    } &&
    requires (Seq& seq, cursor_t<Seq>& cur) {
        { Iface::inc(seq, cur) } -> std::same_as<cursor_t<Seq>&>;
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
    sequence<Seq> && regular_cursor<cursor_t<Seq>> &&
    !detail::disable_multipass<detail::iface_t<Seq>>;

namespace detail {

template <typename Seq, typename Iface = sequence_iface<std::remove_cvref_t<Seq>>>
concept bidirectional_sequence_impl =
    multipass_sequence<Seq> &&
    requires (Seq& seq, cursor_t<Seq>& cur) {
        { Iface::dec(seq, cur) } -> std::same_as<cursor_t<Seq>&>;
    };

} // namespace detail

template <typename Seq>
concept bidirectional_sequence = detail::bidirectional_sequence_impl<Seq>;

namespace detail {

template <typename Seq, typename Iface = sequence_iface<std::remove_cvref_t<Seq>>>
concept random_access_sequence_impl =
    bidirectional_sequence<Seq> && ordered_cursor<cursor_t<Seq>> &&
    requires (Seq& seq, cursor_t<Seq>& cur, distance_t<Seq> offset) {
        { Iface::inc(seq, cur, offset) } -> std::same_as<cursor_t<Seq>&>;
    } &&
    requires (Seq& seq, cursor_t<Seq> const& cur) {
        { Iface::distance(seq, cur, cur) } -> std::convertible_to<distance_t<Seq>>;
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
        { Iface::last(seq) } -> std::same_as<cursor_t<Seq>>;
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

namespace detail {

template <typename T, typename R = std::remove_cvref_t<T>>
constexpr bool is_ilist = false;

template <typename T, typename E>
constexpr bool is_ilist<T, std::initializer_list<E>> = true;

}

template <typename Seq>
concept adaptable_sequence =
    sequence<Seq> &&
    (std::is_lvalue_reference_v<Seq> ||
        (std::movable<std::remove_reference_t<Seq>> && !detail::is_ilist<Seq>));

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
    static constexpr auto is_last(Self& self, cursor_t<Self> const& cur)
        -> decltype(self.is_last(cur))
    {
        return self.is_last(cur);
    }

    template <decays_to<T> Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(self.read_at(cur))
    {
        return self.read_at(cur);
    }

    template <decays_to<T> Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur)
        -> decltype(self.inc(cur))
    {
        return self.inc(cur);
    }

    /* bidirectional sequence interface */
    template <decays_to<T> Self>
    static constexpr auto dec(Self& self, cursor_t<Self>& cur)
        -> decltype(self.dec(cur))
    {
        return self.dec(cur);
    }

    /* random-access sequence interface */
    template <decays_to<T> Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur, distance_t<Self> offset)
        -> decltype(self.inc(cur, offset))
    {
        return self.inc(cur, offset);
    }

    template <decays_to<T> Self>
    static constexpr auto distance(Self& self, cursor_t<Self> const& from,
                                   cursor_t<Self> const& to)
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
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(self.move_at(cur))
    {
        return self.move_at(cur);
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


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED
#define FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_SEQUENCE_ACCESS_HPP_INCLUDED
#define FLUX_CORE_SEQUENCE_ACCESS_HPP_INCLUDED



#include <stdexcept>

namespace flux {

namespace detail {

struct first_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const
        noexcept(noexcept(iface_t<Seq>::first(seq))) -> cursor_t<Seq>
    {
        return iface_t<Seq>::first(seq);
    }
};

struct is_last_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        noexcept(noexcept(iface_t<Seq>::is_last(seq, cur))) -> bool
    {
        return iface_t<Seq>::is_last(seq, cur);
    }
};

struct unchecked_read_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        noexcept(noexcept(iface_t<Seq>::read_at(seq, cur))) -> element_t<Seq>
    {
        return iface_t<Seq>::read_at(seq, cur);
    }
};

struct unchecked_inc_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur) const
        noexcept(noexcept(iface_t<Seq>::inc(seq, cur))) -> cursor_t<Seq>&
    {
        return iface_t<Seq>::inc(seq, cur);
    }

    template <random_access_sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur,
                              distance_t<Seq> offset) const
        noexcept(noexcept(iface_t<Seq>::inc(seq, cur, offset))) -> cursor_t<Seq>&
    {
        return iface_t<Seq>::inc(seq, cur, offset);
    }
};

struct unchecked_dec_fn {
    template <bidirectional_sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur) const
        noexcept(noexcept(iface_t<Seq>::dec(seq, cur))) -> cursor_t<Seq>&
    {
        return iface_t<Seq>::dec(seq, cur);
    }
};

struct distance_fn {
    template <multipass_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& from,
                                            cursor_t<Seq> const& to) const
        -> distance_t<Seq>
    {
        if constexpr (random_access_sequence<Seq>) {
            return iface_t<Seq>::distance(seq, from, to);
        } else {
            distance_t<Seq> n = 0;
            auto from_ = from;
            while (from_ != to) {
                unchecked_inc_fn{}(seq, from_);
                ++n;
            }
            return n;
        }
    }
};

struct data_fn {
    template <contiguous_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const
        noexcept(noexcept(iface_t<Seq>::data(seq)))
    {
        return iface_t<Seq>::data(seq);
    }
};

struct last_fn {
    template <bounded_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const
        noexcept(noexcept(iface_t<Seq>::last(seq))) -> cursor_t<Seq>
    {
        return iface_t<Seq>::last(seq);
    }
};

struct size_fn {
    template <sized_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const -> distance_t<Seq>
    {
        if constexpr (requires { iface_t<Seq>::size(seq); }) {
            return iface_t<Seq>::size(seq);
        } else {
            static_assert(bounded_sequence<Seq> && random_access_sequence<Seq>);
            return distance_fn{}(seq, first_fn{}(seq), last_fn{}(seq));
        }
    }
};

struct unchecked_move_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        -> rvalue_element_t<Seq>
    {
        if constexpr (requires { iface_t<Seq>::move_at(seq, cur); }) {
            return iface_t<Seq>::move_at(seq, cur);
        } else {
            if constexpr (std::is_lvalue_reference_v<element_t<Seq>>) {
                return std::move(unchecked_read_at_fn{}(seq, cur));
            } else {
                return unchecked_read_at_fn{}(seq, cur);
            }
        }
    }
};

struct check_bounds_fn {
    template <typename Seq>
    [[nodiscard]]
    constexpr bool operator()(Seq& seq, cursor_t<Seq> const& cur) const
    {
        if constexpr (random_access_sequence<Seq>) {
            distance_t<Seq> dist = distance_fn{}(seq, first_fn{}(seq), cur);
            if (dist < distance_t<Seq>{0}) {
                return false;
            }
            if constexpr (sized_sequence<Seq>) {
                if (dist >= size_fn{}(seq)) {
                    return false;
                }
            }
        }
        return !is_last_fn{}(seq, cur);
    }
};

inline constexpr auto check_bounds = check_bounds_fn{};

struct checked_read_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        -> decltype(unchecked_read_at_fn{}(seq, cur))
    {
        if (!check_bounds(seq, cur)) {
            throw std::out_of_range("Read via an out-of-bounds cursor");
        }
        return unchecked_read_at_fn{}(seq, cur);
    }
};

struct checked_move_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
            -> decltype(unchecked_move_at_fn{}(seq, cur))
    {
        if (!check_bounds(seq, cur)) {
            throw std::out_of_range("Read via an out-of-bounds cursor");
        }
        return unchecked_move_at_fn{}(seq, cur);
    }
};

struct checked_inc_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur) const
        -> cursor_t<Seq>&
    {
        if (!check_bounds(seq, cur)) {
            throw std::out_of_range("Increment would result in an out-of-bounds cursor");
        }
        return unchecked_inc_fn{}(seq, cur);
    }

    template <random_access_sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur, distance_t<Seq> offset) const
        -> cursor_t<Seq>&
    {
        const auto dist = distance_fn{}(seq, first_fn{}(seq), cur);
        if (dist + offset < 0) {
            throw std::out_of_range("Increment with offset would result in an out-of-bounds cursor");
        }
        if constexpr (sized_sequence<Seq>) {
            if (dist + offset > size_fn{}(seq)) {
                throw std::out_of_range("Increment with offset would result in an out-of-bounds cursor");
            }
        }

        return unchecked_inc_fn{}(seq, cur, offset);
    }
};

struct checked_dec_fn {
    template <bidirectional_sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur) const
        -> cursor_t<Seq>&
    {
        if (cur == first_fn{}(seq)) {
            throw std::out_of_range("Decrement would result in a before-the-start cursor");
        }
        return unchecked_dec_fn{}(seq, cur);
    }
};

} // namespace detail

inline constexpr auto first = detail::first_fn{};
inline constexpr auto is_last = detail::is_last_fn{};
inline constexpr auto unchecked_read_at = detail::unchecked_read_at_fn{};
inline constexpr auto unchecked_move_at = detail::unchecked_move_at_fn{};
inline constexpr auto unchecked_inc = detail::unchecked_inc_fn{};
inline constexpr auto unchecked_dec = detail::unchecked_dec_fn{};
inline constexpr auto distance = detail::distance_fn{};
inline constexpr auto data = detail::data_fn{};
inline constexpr auto last = detail::last_fn{};
inline constexpr auto size = detail::size_fn{};
inline constexpr auto checked_read_at = detail::checked_read_at_fn{};
inline constexpr auto checked_move_at = detail::checked_read_at_fn{};
inline constexpr auto checked_inc = detail::checked_inc_fn{};
inline constexpr auto checked_dec = detail::checked_dec_fn{};

inline constexpr auto read_at = unchecked_read_at;
inline constexpr auto move_at = unchecked_move_at;
inline constexpr auto inc = unchecked_inc;
inline constexpr auto dec = unchecked_dec;


namespace detail {

struct next_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> cur) const
        noexcept(noexcept(inc(seq, cur)))
        -> cursor_t<Seq>
    {
        return inc(seq, cur);
    }

    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> cur, distance_t<Seq> offset) const
        -> cursor_t<Seq>
    {
        if constexpr (random_access_sequence<Seq>) {
            return inc(seq, cur, offset);
        } else if constexpr (bidirectional_sequence<Seq>) {
            const auto zero = distance_t<Seq>{0};
            if (offset > zero) {
                while (offset-- > zero) {
                    inc(seq, cur);
                }
            } else {
                while (offset++ < zero) {
                    dec(seq, cur);
                }
            }
            return cur;
        } else {
            while (offset-- > distance_t<Seq>{0}) {
                inc(seq, cur);
            }
            return cur;
        }
    }
};

struct prev_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> cur) const
        noexcept(noexcept(dec(seq, cur)))
        -> cursor_t<Seq>
    {
        return dec(seq, cur);
    }
};

struct is_empty_fn {
    template <multipass_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const -> bool
    {
        if constexpr (sized_sequence<Seq>) {
            return flux::size(seq) == 0;
        } else {
            return is_last(seq, first(seq));
        }
    }
};

template <typename Seq1, typename Seq2>
concept element_swappable_with_ =
    std::constructible_from<value_t<Seq1>, rvalue_element_t<Seq1>> &&
    writable_sequence_of<Seq1, rvalue_element_t<Seq2>> &&
    writable_sequence_of<Seq2, value_t<Seq1>&&>;

template <typename Seq1, typename Seq2>
concept element_swappable_with =
    element_swappable_with_<Seq1, Seq2> &&
    element_swappable_with_<Seq2, Seq1>;

struct swap_with_fn {
    template <sequence Seq1, sequence Seq2>
    constexpr void operator()(Seq1& seq1, cursor_t<Seq1> const& cur1,
                              Seq2& seq2, cursor_t<Seq2> const& cur2) const
        requires (std::swappable_with<element_t<Seq1>, element_t<Seq2>> ||
                  element_swappable_with<Seq1, Seq2>)
    {
        if constexpr (std::swappable_with<element_t<Seq1>, element_t<Seq2>>) {
            return std::ranges::swap(read_at(seq1, cur1), read_at(seq2, cur2));
        } else {
            value_t<Seq1> temp(move_at(seq1, cur1));
            read_at(seq1, cur1) = move_at(seq2, cur2);
            read_at(seq2, cur2) = std::move(temp);
        }
    }
};

struct swap_at_fn {
    template <sequence Seq>
    constexpr void operator()(Seq& seq, cursor_t<Seq> const& first,
                              cursor_t<Seq> const& second) const
        requires requires { swap_with_fn{}(seq, first, seq, second); }
    {
        return swap_with_fn{}(seq, first, seq, second);
    }
};

} // namespace detail


inline constexpr auto next = detail::next_fn{};
inline constexpr auto prev = detail::prev_fn{};
inline constexpr auto is_empty = detail::is_empty_fn{};
inline constexpr auto swap_with = detail::swap_with_fn{};
inline constexpr auto swap_at = detail::swap_at_fn{};

} // namespace flux

#endif


#include <initializer_list>
#include <functional>

namespace flux {

/*
 * Default implementation for C arrays of known bound
 */
template <typename T, std::size_t N>
struct sequence_iface<T[N]> {

    static constexpr auto first(auto const&) -> std::size_t { return 0; }

    static constexpr bool is_last(auto const&, std::size_t idx) { return idx == N; }

    static constexpr auto read_at(auto& self, std::size_t idx) -> decltype(auto) { return self[idx]; }

    static constexpr auto& inc(auto const&, std::size_t& idx) { return ++idx; }

    static constexpr auto last(auto const&) { return N; }

    static constexpr auto& dec(auto const&, std::size_t& idx) { return --idx; }

    static constexpr auto& inc(auto const&, std::size_t& idx, std::ptrdiff_t offset)
    {
        return idx += offset;
    }

    static constexpr auto distance(auto const&, std::size_t from, std::size_t to)
    {
        return static_cast<std::ptrdiff_t>(to) - static_cast<std::ptrdiff_t>(from);
    }

    static constexpr auto data(auto& self) { return self; }

    static constexpr auto size(auto const&) { return N; }
};

/*
 * Default implementation for std::initializer_list
 */
template <typename T>
struct sequence_iface<std::initializer_list<T>> {

    using ilist_t = std::initializer_list<T>;

    static constexpr auto first(ilist_t self) -> T const* { return self.begin(); }

    static constexpr auto is_last(ilist_t self, T const* ptr) -> bool
    {
        return ptr == self.end();
    }

    static constexpr auto read_at(ilist_t, T const* ptr) -> T const& { return *ptr; }

    static constexpr auto inc(ilist_t, T const*& ptr) -> T const*& { return ++ptr; }

    static constexpr auto last(ilist_t self) -> T const* { return self.end(); }

    static constexpr auto dec(ilist_t, T const*& ptr) -> T const*& { return --ptr; }

    static constexpr auto inc(ilist_t, T const*& ptr, std::ptrdiff_t offset) -> T const*&
    {
        return ptr += offset;
    }

    static constexpr auto distance(ilist_t, T const* from, T const* to) -> std::ptrdiff_t
    {
        return to - from;
    }

    static constexpr auto data(ilist_t self) -> T const* { return std::data(self); }

    static constexpr auto size(ilist_t self) { return self.size(); }
};

/*
 * Default implementation for std::reference_wrapper<T>
 */
template <sequence Seq>
struct sequence_iface<std::reference_wrapper<Seq>> {

    using self_t = std::reference_wrapper<Seq>;

    using value_type = value_t<Seq>;
    using distance_type = distance_t<Seq>;

    static constexpr bool disable_multipass = !multipass_sequence<Seq>;

    static constexpr auto first(self_t self) -> cursor_t<Seq>
    {
         return flux::first(self.get());
    }

    static constexpr bool is_last(self_t self, cursor_t<Seq> const& cur)
    {
        return flux::is_last(self.get(), cur);
    }

    static constexpr auto read_at(self_t self, cursor_t<Seq> const& cur)
        -> decltype(auto)
    {
        return flux::read_at(self.get(), cur);
    }

    static constexpr auto inc(self_t self, cursor_t<Seq>& cur)
        -> cursor_t<Seq>&
    {
        return flux::inc(self.get(), cur);
    }

    static constexpr auto dec(self_t self, cursor_t<Seq>& cur)
        -> cursor_t<Seq>&
        requires bidirectional_sequence<Seq>
    {
        return flux::dec(self.get(), cur);
    }

    static constexpr auto inc(self_t self, cursor_t<Seq>& cur, distance_type offset)
        -> cursor_t<Seq>&
        requires random_access_sequence<Seq>
    {
        return flux::inc(self.get(), cur,  offset);
    }

    static constexpr auto distance(self_t self, cursor_t<Seq> const& from,
                                   cursor_t<Seq> const& to)
        -> distance_type
        requires random_access_sequence<Seq>
    {
        return flux::distance(self.get(), from, to);
    }

    static constexpr auto data(self_t self)
        requires contiguous_sequence<Seq>
    {
        return flux::data(self.get());
    }

    static constexpr auto last(self_t self) -> cursor_t<Seq>
        requires bounded_sequence<Seq>
    {
        return flux::last(self.get());
    }

    static constexpr auto size(self_t self) -> distance_type
        requires sized_sequence<Seq>
    {
        return flux::size(self.get());
    }

    static constexpr auto move_at(self_t self, cursor_t<Seq> const& cur)
        -> rvalue_element_t<Seq>
    {
        return flux::move_at(self.get(), cur);
    }
};

} // namespace flux

#endif // FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_LENS_BASE_HPP_INCLUDED
#define FLUX_CORE_LENS_BASE_HPP_INCLUDED



namespace flux {

template <cursor Cur>
struct bounds {
    FLUX_NO_UNIQUE_ADDRESS Cur from;
    FLUX_NO_UNIQUE_ADDRESS Cur to;

    friend bool operator==(bounds const&, bounds const&) = default;
};

template <sequence Seq>
using bounds_t = bounds<cursor_t<Seq>>;

template <typename Derived>
    /*requires std::is_class_v<Derived>&&
             std::same_as<Derived, std::remove_cv_t<Derived>>*/
struct lens_base {
private:
    constexpr auto derived() -> Derived& { return static_cast<Derived&>(*this); }
    constexpr auto derived() const -> Derived const& { return static_cast<Derived const&>(*this); }

protected:
    ~lens_base() = default;

public:
    /*
     * Basic iteration functions
     */

    /// Returns a cursor pointing to the first element of the sequence
    [[nodiscard]]
    constexpr auto first() { return flux::first(derived()); }

    /// Returns true if `cur` points to the end of the sequence
    ///
    /// @param cur The cursor to test
    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr bool is_last(cursor_t<D> const& cur) { return flux::is_last(derived(), cur); }

    /// Increments the given cursor
    ///
    /// @param cur the cursor to increment
    template <std::same_as<Derived> D = Derived>
    constexpr auto& inc(cursor_t<D>& cur) { return flux::inc(derived(), cur); }

    /// Returns the element at the given cursor
    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr decltype(auto) read_at(cursor_t<D> const& cur) { return flux::read_at(derived(), cur); }

    /// Returns an rvalue version of the element at the given cursor
    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr decltype(auto) move_at(cursor_t<D> const& cur) { return flux::move_at(derived(), cur); }

    /// Returns the element at the given cursor
    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr decltype(auto) operator[](cursor_t<D> const& cur) { return flux::read_at(derived(), cur); }

    /// Returns an cursor pointing to one past the last element of the sequence
    [[nodiscard]]
    constexpr auto last() requires bounded_sequence<Derived> { return flux::last(derived()); }

    /// Decrements the given cursor
    template <std::same_as<Derived> D = Derived>
        requires bidirectional_sequence<Derived>
    constexpr auto& dec(cursor_t<D>& cur) { return flux::dec(derived(), cur); }

    /// Increments the given cursor by `offset` places
    template <std::same_as<Derived> D = Derived>
        requires random_access_sequence<Derived>
    constexpr auto& inc(cursor_t<D>& cur, distance_t<D> offset) { return flux::inc(derived(), cur, offset); }

    /// Returns the number of times `from` must be incremented to reach `to`
    ///
    /// For a random-access sequence, returns the result in constant time
    template <std::same_as<Derived> D = Derived>
        requires multipass_sequence<Derived>
    [[nodiscard]]
    constexpr auto distance(cursor_t<D> const& from, cursor_t<D> const& to)
    {
        return flux::distance(derived(), from, to);
    }

    [[nodiscard]]
    constexpr auto data() requires contiguous_sequence<Derived>
    {
        return flux::data(derived());
    }

    /// Returns the number of elements in the sequence
    [[nodiscard]]
    constexpr auto size() requires sized_sequence<Derived> { return flux::size(derived()); }

    /// Returns true of the sequence contains no elements
    [[nodiscard]]
    constexpr auto is_empty() requires multipass_sequence<Derived> { return flux::is_empty(derived()); }

    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr auto next(cursor_t<D> cur) { return flux::next(derived(), cur); }

    template <std::same_as<Derived> D = Derived>
        requires bidirectional_sequence<Derived>
    [[nodiscard]]
    constexpr auto prev(cursor_t<D> cur) { return flux::prev(derived(), cur); }

    /*
     * Adaptors
     */

    constexpr auto cache_last() &&
            requires bounded_sequence<Derived> ||
                     (multipass_sequence<Derived> && not infinite_sequence<Derived>);

    template <std::same_as<Derived> D = Derived>
    constexpr auto drop(distance_t<D> count) &&;

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    constexpr auto drop_while(Pred pred) &&;

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>&>
    [[nodiscard]]
    constexpr auto filter(Pred pred) &&;

    template <typename Func>
        requires std::invocable<Func&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto map(Func func) &&;

    [[nodiscard]]
    constexpr auto reverse() &&
            requires bidirectional_sequence<Derived> && bounded_sequence<Derived>;

    template <multipass_sequence Pattern>
        requires std::equality_comparable_with<element_t<Derived>, element_t<Pattern>>
    [[nodiscard]]
    constexpr auto split(Pattern&& pattern) &&;

    template <typename ValueType>
        requires decays_to<ValueType, value_t<Derived>>
    [[nodiscard]]
    constexpr auto split(ValueType&& delim) &&;

    template <typename Pattern>
    [[nodiscard]]
    constexpr auto split_string(Pattern&& pattern) &&;

    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr auto take(distance_t<D> count) &&;

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto take_while(Pred pred) &&;

    [[nodiscard]]
    constexpr auto view() &;

    [[nodiscard]]
    constexpr auto view() const& requires sequence<Derived const>;

    [[nodiscard]]
    constexpr auto view() &&;

    [[nodiscard]]
    constexpr auto view() const&& requires sequence<Derived const>;

    /*
     * Folds of various kinds
     */

    /// Returns `true` if every element of the sequence satisfies the predicate
    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    [[nodiscard]]
    constexpr auto all(Pred pred, Proj proj = {});

    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    [[nodiscard]]
    constexpr auto any(Pred pred, Proj proj = {});

    template <typename Value, typename Proj = std::identity>
        requires std::equality_comparable_with<projected_t<Proj, Derived>, Value const&>
    constexpr auto contains(Value const& value, Proj proj = {}) -> bool;

    /// Returns the number of elements in the sequence
    constexpr auto count();

    /// Returns the number of elements in the sequence which are equal to `value`
    template <typename Value, typename Proj = std::identity>
        requires std::equality_comparable_with<projected_t<Proj, Derived>, Value const&>
    constexpr auto count(Value const& value, Proj proj = {});

    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    constexpr auto count_if(Pred pred, Proj proj = {});

    /// Returns a cursor pointing to the first occurrence of `value` in the sequence
    template <typename Value, typename Proj = std::identity>
        requires std::equality_comparable_with<projected_t<Proj, Derived>, Value const&>
    [[nodiscard]]
    constexpr auto find(Value const&, Proj proj = {});

    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    [[nodiscard]]
    constexpr auto find_if(Pred pred, Proj proj = {});

    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    [[nodiscard]]
    constexpr auto find_if_not(Pred pred, Proj proj = {});

    template <typename Func, typename Proj = std::identity>
        requires std::invocable<Func&, projected_t<Proj, Derived>>
    constexpr auto for_each(Func func, Proj proj = {}) -> Func;

    template <typename Pred>
        requires std::invocable<Pred&, element_t<Derived>> &&
                 detail::boolean_testable<std::invoke_result_t<Pred&, element_t<Derived>>>
    constexpr auto for_each_while(Pred pred);

    constexpr auto inplace_reverse()
        requires bounded_sequence<Derived> &&
                 detail::element_swappable_with<Derived, Derived>;

    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    [[nodiscard]]
    constexpr auto none(Pred pred, Proj proj = {});

    template <typename Iter>
        requires std::weakly_incrementable<Iter> &&
                 std::indirectly_writable<Iter, element_t<Derived>>
    constexpr auto output_to(Iter iter) -> Iter;

    auto write_to(std::ostream& os) -> std::ostream&;
};


} // namespace flux

#endif // FLUX_CORE_LENS_BASE_HPP_INCLUDED




#endif // FLUX_CORE_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ALL_ANY_NONE_HPP_INCLUDED
#define FLUX_OP_ALL_ANY_NONE_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FOR_EACH_WHILE_HPP_INCLUDED
#define FLUX_OP_FOR_EACH_WHILE_HPP_INCLUDED



namespace flux {

namespace detail {

struct for_each_while_fn {
    template <sequence Seq, typename Pred>
        requires std::invocable<Pred&, element_t<Seq>> &&
                 boolean_testable<std::invoke_result_t<Pred&, element_t<Seq>>>
    constexpr auto operator()(Seq&& seq, Pred pred) const -> cursor_t<Seq>
    {
        if constexpr (requires { iface_t<Seq>::for_each_while(seq, std::move(pred)); }) {
            return iface_t<Seq>::for_each_while(seq, std::move(pred));
        } else {
            auto cur = first(seq);
            while (!is_last(seq, cur)) {
                if (!std::invoke(pred, read_at(seq, cur))) { break; }
                inc(seq, cur);
            }
            return cur;
        }
    }
};

} // namespace detail

inline constexpr auto for_each_while = detail::for_each_while_fn{};

template <typename Derived>
template <typename Pred>
    requires std::invocable<Pred&, element_t<Derived>> &&
             detail::boolean_testable<std::invoke_result_t<Pred&, element_t<Derived>>>
constexpr auto lens_base<Derived>::for_each_while(Pred pred)
{
    return flux::for_each_while(derived(), std::ref(pred));
}

} // namespace flux

#endif // FLUX_OP_FOR_EACH_WHILE_HPP_INCLUDED


namespace flux {

namespace all_detail {

struct fn {
    template <sequence Seq, typename Proj = std::identity,
              predicate_for<Seq, Proj> Pred>
    constexpr bool operator()(Seq&& seq, Pred pred, Proj proj = {}) const
    {
        return is_last(seq, for_each_while(seq, [&](auto&& elem) {
            return std::invoke(pred, std::invoke(proj, FLUX_FWD(elem)));
        }));
    }
};

} // namespace all_detail

inline constexpr auto all = all_detail::fn{};

namespace none_detail {

struct fn {
    template <sequence Seq, typename Proj = std::identity,
              predicate_for<Seq, Proj> Pred>
    constexpr bool operator()(Seq&& seq, Pred pred, Proj proj = {}) const
    {
        return is_last(seq, for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, std::invoke(proj, FLUX_FWD(elem)));
        }));
    }
};

} // namespace none_detail

inline constexpr auto none = none_detail::fn{};

namespace any_detail {

struct fn {
    template <sequence Seq, typename Proj = std::identity,
              predicate_for<Seq, Proj> Pred>
    constexpr bool operator()(Seq&& seq, Pred pred, Proj proj = {}) const
    {
        return !is_last(seq, for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, std::invoke(proj, FLUX_FWD(elem)));
        }));
    }
};

} // namespace any_detail

inline constexpr auto any = any_detail::fn{};

template <typename D>
template <typename Pred, typename Proj>
    requires predicate_for<Pred, D, Proj>
constexpr auto lens_base<D>::all(Pred pred, Proj proj)
{
    return flux::all(derived(), std::move(pred), std::move(proj));
}

template <typename D>
template <typename Pred, typename Proj>
    requires predicate_for<Pred, D, Proj>
constexpr auto lens_base<D>::any(Pred pred, Proj proj)
{
    return flux::any(derived(), std::move(pred), std::move(proj));
}

template <typename D>
template <typename Pred, typename Proj>
    requires predicate_for<Pred, D, Proj>
constexpr auto lens_base<D>::none(Pred pred, Proj proj)
{
    return flux::none(derived(), std::move(pred), std::move(proj));
}

} // namespace flux

#endif // FLUX_OP_ALL_ANY_NONE_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_BOUNDS_CHECKED_HPP_INCLUDED
#define FLUX_OP_BOUNDS_CHECKED_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FROM_HPP_INCLUDED
#define FLUX_OP_FROM_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_REF_HPP_INCLUDED
#define FLUX_OP_REF_HPP_INCLUDED





namespace flux {

namespace detail {

template <sequence Base>
struct ref_adaptor : lens_base<ref_adaptor<Base>> {
private:
    Base* base_;

    static void test_func(Base&) noexcept;
    static void test_func(Base&&) = delete;

public:
    // This seems thoroughly overcomplicated, but it's the formulation
    // std::reference_wrapper and ranges::ref_view use to avoid binding rvalues
    // when Base is a const type, while also avoiding implicit conversions
    template <typename Seq>
        requires (!std::same_as<std::decay_t<Seq>, ref_adaptor> &&
                  std::convertible_to<Seq, Base&> &&
                  requires { test_func(std::declval<Seq>()); })
    constexpr ref_adaptor(Seq&& seq)
        noexcept(noexcept(test_func(std::declval<Seq>())))
        : base_(std::addressof(static_cast<Base&>(FLUX_FWD(seq))))
    {}

    constexpr Base& base() const noexcept { return *base_; }

    friend struct sequence_iface<ref_adaptor>;
};

template <typename>
inline constexpr bool is_ref_adaptor = false;

template <typename T>
inline constexpr bool is_ref_adaptor<ref_adaptor<T>> = true;

struct ref_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const
    {
        if constexpr (is_ref_adaptor<Seq>) {
            return seq;
        } else {
            return ref_adaptor<Seq>(seq);
        }
    }
};

template <sequence Base>
    requires std::movable<Base>
struct owning_adaptor : lens_base<owning_adaptor<Base>> {
private:
    Base base_;

public:
    constexpr explicit owning_adaptor(Base&& base)
        : base_(std::move(base))
    {}

    owning_adaptor(owning_adaptor&&) = default;
    owning_adaptor& operator=(owning_adaptor&&) = default;

    constexpr Base& base() & noexcept { return base_; }
    constexpr Base const& base() const& noexcept { return base_; }
    constexpr Base&& base() && noexcept { return base_; }
    constexpr Base const&& base() const&& noexcept { return base_; }

    friend struct sequence_iface<owning_adaptor>;
};

template <sequence Base>
struct passthrough_iface_base {

    template <typename Self>
    using cursor_t = decltype(flux::first(FLUX_DECLVAL(Self&).base()));

    using value_type = value_t<Base>;
    using distance_type = distance_t<Base>;

    static constexpr bool disable_multipass = !multipass_sequence<Base>;
    static constexpr bool is_infinite = infinite_sequence<Base>;

    static constexpr auto first(auto& self)
        -> decltype(flux::first(self.base()))
    {
        return flux::first(self.base());
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, cursor_t<Self> const& cur)
        -> decltype(flux::is_last(self.base(), cur))
    {
        return flux::is_last(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(flux::read_at(self.base(), cur))
    {
        return flux::read_at(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur)
        -> decltype(flux::inc(self.base(), cur))
    {
        return flux::inc(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto dec(Self& self, cursor_t<Self>& cur)
        -> decltype(flux::dec(self.base(), cur))
    {
        return flux::dec(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur, distance_type dist)
        -> decltype(flux::inc(self.base(), cur, dist))
    {
        return flux::inc(self.base(), cur, dist);
    }

    template <typename Self>
    static constexpr auto distance(Self& self, cursor_t<Self> const& from, cursor_t<Self> const& to)
        -> decltype(flux::distance(self.base(), from, to))
        requires random_access_sequence<decltype(self.base())>
    {
        return flux::distance(self.base(), from, to);
    }

    static constexpr auto data(auto& self)
        -> decltype(flux::data(self.base()))
    {
        return flux::data(self.base());
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        -> decltype(flux::size(self.base()))
        requires sized_sequence<decltype(self.base())>
    {
        return flux::size(self.base());
    }

    static constexpr auto last(auto& self)
        -> decltype(flux::last(self.base()))
    {
        return flux::last(self.base());
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, auto const& cur)
        -> decltype(flux::move_at(self.base(), cur))
    {
        return flux::move_at(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto for_each_while(Self& self, auto&& pred)
        -> decltype(flux::for_each_while(self.base(), FLUX_FWD(pred)))
    {
        return flux::for_each_while(self.base(), FLUX_FWD(pred));
    }

    template <typename Self>
    static constexpr auto slice(Self& self, cursor_t<Self> from, cursor_t<Self> to)
        -> decltype(iface_t<decltype(self.base())>::slice(self.base(), std::move(from), std::move(to)))
    {
        return iface_t<decltype(self.base())>::slice(self.base(), std::move(from), std::move(to));
    }

    template <typename Self>
    static constexpr auto slice(Self& self, cursor_t<Self> from)
        -> decltype(iface_t<decltype(self.base())>::slice(self.base(), std::move(from)))
    {
        return iface_t<decltype(self.base())>::slice(self.base(), std::move(from));
    }
};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::ref_adaptor<Base>>
    : detail::passthrough_iface_base<Base> {};

template <typename Base>
struct sequence_iface<detail::owning_adaptor<Base>>
    : detail::passthrough_iface_base<Base> {};

inline constexpr auto ref = detail::ref_fn{};

/*
template <typename D>
constexpr auto lens_base<D>::ref() & -> lens auto
{
    return detail::ref_adaptor<D>(derived());
}
*/

} // namespace flux

#endif // FLUX_OP_REF_HPP_INCLUDED


namespace flux {

namespace detail {

struct from_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> lens auto
    {
        if constexpr (lens<std::remove_cvref_t<Seq>>) {
            if constexpr (std::is_lvalue_reference_v<Seq>) {
                return flux::ref(seq);
            } else {
                return FLUX_FWD(seq);
            }
        } else if constexpr (std::is_lvalue_reference_v<Seq>) {
            return detail::ref_adaptor<std::remove_reference_t<Seq>>(seq);
        } else {
            return detail::owning_adaptor<Seq>(FLUX_FWD(seq));
        }
    }
};

// This should go elsewhere...
template <typename Seq>
using lens_t = decltype(from_fn{}(FLUX_DECLVAL(Seq)));

} // namespace detail

inline constexpr auto from = detail::from_fn{};

} // namespace flux

#endif // FLUX_OP_FROM_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SLICE_HPP_INCLUDED
#define FLUX_OP_SLICE_HPP_INCLUDED




namespace flux {

namespace detail {

template <cursor Cur, bool Bounded>
struct slice_data {
    Cur first;
    Cur last;
};

template <cursor Cur>
struct slice_data<Cur, false> {
    Cur first;
};

template <lens Base, bool Bounded>
    requires (!Bounded || regular_cursor<cursor_t<Base>>)
struct subsequence : lens_base<subsequence<Base, Bounded>>
{
private:
    Base* base_;
    FLUX_NO_UNIQUE_ADDRESS slice_data<cursor_t<Base>, Bounded> data_;

    friend struct sequence_iface<subsequence>;

public:
    constexpr subsequence(Base& base, cursor_t<Base>&& from,
                          cursor_t<Base>&& to)
        requires Bounded
        : base_(std::addressof(base)),
          data_{std::move(from), std::move(to)}
    {}

    constexpr subsequence(Base& base, cursor_t<Base>&& from)
        requires (!Bounded)
        : base_(std::addressof(base)),
          data_{std::move(from)}
    {}

    constexpr auto base() const -> Base& { return *base_; };
};

template <sequence Seq>
subsequence(Seq&, cursor_t<Seq>, cursor_t<Seq>) -> subsequence<Seq, true>;

template <sequence Seq>
subsequence(Seq&, cursor_t<Seq>) -> subsequence<Seq, false>;

template <typename Seq>
concept has_overloaded_slice =
    requires (Seq& seq, cursor_t<Seq> cur) {
        { iface_t<Seq>::slice(seq, std::move(cur)) } -> sequence;
        { iface_t<Seq>::slice(seq, std::move(cur), std::move(cur)) } -> sequence;
    };

struct slice_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq> from,
                              cursor_t<Seq> to) const
        -> sequence auto
    {
        if constexpr (has_overloaded_slice<Seq>) {
            return iface_t<Seq>::slice(seq, std::move(from), std::move(to));
        } else {
            return subsequence(seq, std::move(from), std::move(to));
        }
    }

    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq> from, last_fn) const
        -> sequence auto
    {
        if constexpr (has_overloaded_slice<Seq>) {
            return iface_t<Seq>::slice(seq, std::move(from));
        } else {
            return subsequence(seq, std::move(from));
        }
    }
};

} // namespace detail

using detail::subsequence;

template <typename Base, bool Bounded>
struct sequence_iface<subsequence<Base, Bounded>>
    : detail::passthrough_iface_base<Base>
{
    using self_t = subsequence<Base, Bounded>;

    static constexpr auto first(self_t& self) -> cursor_t<Base>
    {
        if constexpr (std::copy_constructible<decltype(self.data_.first)>) {
            return self.data_.first;
        } else {
            return std::move(self.data_.first);
        }
    }

    static constexpr bool is_last(self_t& self, cursor_t<Base> const& cur) {
        if constexpr (Bounded) {
            return cur == self.data_.last;
        } else {
            return flux::is_last(*self.base_, cur);
        }
    }

    static constexpr auto last(self_t& self) -> cursor_t<Base>
        requires (Bounded || bounded_sequence<Base>)
    {
        if constexpr (Bounded) {
            return self.data_.last;
        } else {
            return flux::last(*self.base_);
        }
    }

    static constexpr auto data(self_t& self)
        requires contiguous_sequence<Base>
    {
        return flux::data(*self.base_) +
               flux::distance(*self.base_, flux::first(*self.base_), self.data_.first);
    }

    void size() = delete;
    void for_each_while() = delete;
};

inline constexpr auto slice = detail::slice_fn{};

#if 0
template <typename Derived>
template <std::same_as<Derived> D>
constexpr auto lens_base<Derived>::slice(cursor_t<D> from, cursor_t<D> to) &
{
    return flux::slice(derived(), std::move(from), std::move(to));
}

template <typename Derived>
template <std::same_as<Derived> D>
constexpr auto lens_base<Derived>::slice_from(cursor_t<D> from) &
{
    return flux::slice(derived(), std::move(from));
}
#endif

} // namespace flux

#endif // namespace FLUX_OP_SLICE_HPP_INCLUDED


namespace flux {

namespace detail {

template <lens Base>
struct bounds_checked_adaptor : lens_base<bounds_checked_adaptor<Base>>
{
private:
    Base base_;

    friend struct sequence_iface<bounds_checked_adaptor>;

public:
    constexpr explicit bounds_checked_adaptor(Base&& base)
        : base_(std::move(base))
    {}

    constexpr auto base() -> Base& { return base_; }
    constexpr auto base() const -> Base const& { return base_; }
};

struct bounds_checked_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        return bounds_checked_adaptor(flux::from(FLUX_FWD(seq)));
    }
};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::bounds_checked_adaptor<Base>>
    : detail::passthrough_iface_base<Base>
{
    static constexpr auto read_at(auto& self, auto const& cur)
        -> decltype(flux::checked_read_at(self.base_, cur))
    {
        return flux::checked_read_at(self.base_, cur);
    }

    static constexpr auto inc(auto& self, auto& cur)
        -> decltype(flux::checked_inc(self.base_, cur))
    {
        return flux::checked_inc(self.base_, cur);
    }

    static constexpr auto dec(auto& self, auto& cur)
        -> decltype(flux::checked_dec(self.base_, cur))
    {
        return flux::checked_dec(self.base_, cur);
    }

    static constexpr auto inc(auto& self, auto& cur, auto offset)
        -> decltype(flux::checked_inc(self.base_, cur, offset))
    {
        return flux::checked_inc(self.base_, cur, offset);
    }

    static constexpr auto move_at(auto& self, auto const& cur)
        -> decltype(flux::checked_move_at(self.base_, cur))
    {
        return flux::checked_move_at(self.base_, cur);
    }

    static constexpr auto slice(auto& self, auto first, auto last)
    {
        return detail::bounds_checked_adaptor(
            flux::from(flux::slice(self.base_, std::move(first), std::move(last))));
    }

    static constexpr auto slice(auto& self, auto first)
    {
        return detail::bounds_checked_adaptor(flux::from(flux::slice(self.base_, std::move(first), flux::last)));
    }
};

inline constexpr auto bounds_checked = detail::bounds_checked_fn{};

} // namespace flux

#endif // FLUX_OP_BOUNDS_CHECKED_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CACHE_LAST_HPP_INCLUDED
#define FLUX_OP_CACHE_LAST_HPP_INCLUDED




#include <optional>

namespace flux {

namespace detail {

template <lens Base>
struct cache_last_adaptor : lens_base<cache_last_adaptor<Base>>
{
private:
    Base base_;
    std::optional<cursor_t<Base>> cached_last_{};

    friend struct sequence_iface<cache_last_adaptor>;
    friend struct passthrough_iface_base<Base>;

    constexpr auto base() -> Base& { return base_; }

public:
    constexpr explicit cache_last_adaptor(Base&& base)
        : base_(std::move(base))
    {}
};

struct cache_last_fn {
    template <adaptable_sequence Seq>
        requires bounded_sequence<Seq> ||
            (multipass_sequence<Seq> && not infinite_sequence<Seq>)
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr (bounded_sequence<Seq>) {
            return flux::from(FLUX_FWD(seq));
        } else {
            return cache_last_adaptor(flux::from(FLUX_FWD(seq)));
        }
    }
};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::cache_last_adaptor<Base>>
    : detail::passthrough_iface_base<Base> {

    using self_t = detail::cache_last_adaptor<Base>;

    static constexpr auto is_last(self_t& self, cursor_t<self_t> const& cur)
    {
        if (flux::is_last(self.base_, cur)) {
            self.cached_last_ = cur;
            return true;
        } else {
            return false;
        }
    }

    static constexpr auto last(self_t& self)
    {
        if (!self.cached_last_) {
            auto cur = flux::first(self);
            while (!is_last(self, cur)) {
                flux::inc(self.base_, cur);
            }
            assert(self.cached_last_.has_value());
        }
        return *self.cached_last_;
    }
};

inline constexpr auto cache_last = detail::cache_last_fn{};

template <typename Derived>
constexpr auto lens_base<Derived>::cache_last() &&
    requires bounded_sequence<Derived> ||
        (multipass_sequence<Derived> && not infinite_sequence<Derived>)
{
    return flux::cache_last(std::move(derived()));
}

} // namespace flux

#endif // FLUX_OP_CACHE_LAST_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CHAIN_HPP_INCLUDED
#define FLUX_OP_CHAIN_HPP_INCLUDED



#include <cassert>
#include <tuple>
#include <variant>

namespace flux {

namespace detail {

template <lens... Bases>
struct chain_adaptor : lens_base<chain_adaptor<Bases...>> {
private:
    std::tuple<Bases...> bases_;

    friend struct sequence_iface<chain_adaptor>;

public:
    explicit constexpr chain_adaptor(Bases&&... bases)
        : bases_(std::move(bases)...)
    {}
};

template <typename... Ts>
concept all_have_common_ref =
    requires { typename std::common_reference_t<Ts...>; } &&
    (std::convertible_to<Ts, std::common_reference_t<Ts...>> && ...);

template <typename... Seqs>
concept chainable =
    all_have_common_ref<element_t<Seqs>...> &&
    all_have_common_ref<rvalue_element_t<Seqs>...> &&
    requires { typename std::common_type_t<value_t<Seqs>...>; };

struct chain_fn {
    template <adaptable_sequence... Seqs>
        requires (sizeof...(Seqs) >= 1) &&
                 chainable<Seqs...>
    constexpr auto operator()(Seqs&&... seqs) const
    {
        if constexpr (sizeof...(Seqs) == 1) {
            return flux::from(FLUX_FWD(seqs)...);
        } else {
            return chain_adaptor(flux::from(FLUX_FWD(seqs))...);
        }
    }
};

} // namespace detail

template <typename... Bases>
struct sequence_iface<detail::chain_adaptor<Bases...>> {

    using value_type = std::common_type_t<value_t<Bases>...>;
    using distance_type = std::common_type_t<distance_t<Bases>...>;

    static constexpr bool disable_multipass = !(multipass_sequence<Bases> && ...);
    static constexpr bool is_infinite = (infinite_sequence<Bases> || ...);

private:
    static constexpr std::size_t End = sizeof...(Bases) - 1;

    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    template <typename Self>
    using cursor_type = std::variant<cursor_t<const_like_t<Self, Bases>>...>;

    template <std::size_t N, typename Self>
    static constexpr auto first_impl(Self& self)
        -> cursor_type<Self>
    {
        auto& base = std::get<N>(self.bases_);
        auto cur = flux::first(base);

        if constexpr (N < End) {
            if (!flux::is_last(base, cur)) {
                return cursor_type<Self>(std::in_place_index<N>, std::move(cur));
            } else {
                return first_impl<N+1>(self);
            }
        } else {
            return cursor_type<Self>(std::in_place_index<N>, std::move(cur));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto inc_impl(Self& self, cursor_type<Self>& cur)
            -> cursor_type<Self>&
    {
        if constexpr (N < End) {
            if (cur.index() == N) {
                auto& base = std::get<N>(self.bases_);
                auto& base_cur = std::get<N>(cur);
                flux::inc(base, base_cur);
                if (flux::is_last(base, base_cur)) {
                    cur = first_impl<N + 1>(self);
                }
                return cur;
            } else {
                return inc_impl<N+1>(self, cur);
            }
        } else {
            flux::inc(std::get<N>(self.bases_), std::get<N>(cur));
            return cur;
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto dec_impl(Self& self, cursor_type<Self>& cur)
        -> cursor_type<Self>&
    {
        if constexpr (N > 0) {
            if (cur.index() == N) {
                auto& base = std::get<N>(self.bases_);
                auto& base_cur = std::get<N>(cur);

                if (base_cur == flux::first(base)) {
                    cur = cursor_type<Self>(std::in_place_index<N-1>,
                                           flux::last(std::get<N-1>(self.bases_)));
                    return dec_impl<N-1>(self, cur);
                } else {
                    flux::dec(base, base_cur);
                    return cur;
                }
            } else {
                return dec_impl<N-1>(self, cur);
            }
        } else {
            flux::dec(std::get<0>(self.bases_), std::get<0>(cur));
            return cur;
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto read_impl(Self& self, cursor_type<Self> const& cur)
        -> std::common_reference_t<element_t<const_like_t<Self, Bases>>...>
    {
        if constexpr (N < End) {
            if (cur.index() == N) {
                return flux::read_at(std::get<N>(self.bases_), std::get<N>(cur));
            } else {
                return read_impl<N+1>(self, cur);
            }
        } else {
            return flux::read_at(std::get<N>(self.bases_), std::get<N>(cur));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto move_impl(Self& self, cursor_type<Self> const& cur)
        -> std::common_reference_t<rvalue_element_t<const_like_t<Self, Bases>>...>
    {
        if constexpr (N < End) {
            if (cur.index() == N) {
                return flux::move_at(std::get<N>(self.bases_), std::get<N>(cur));
            } else {
                return move_impl<N+1>(self, cur);
            }
        } else {
            return flux::move_at(std::get<N>(self.bases_), std::get<N>(cur));
        }
    }


    template <std::size_t N, typename Self>
    static constexpr auto for_each_while_impl(Self& self, auto& pred)
        -> cursor_type<Self>
    {
        if constexpr (N < End) {
            auto& base = std::get<N>(self.bases_);
            auto base_cur = flux::for_each_while(base, pred);
            if (!flux::is_last(base, base_cur)) {
                return cursor_type<Self>(std::in_place_index<N>, std::move(base_cur));
            } else {
                return for_each_while_impl<N+1>(self, pred);
            }
        } else {
            return cursor_type<Self>(std::in_place_index<N>,
                                    flux::for_each_while(std::get<N>(self.bases_), pred));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto distance_impl(Self& self,
                                        cursor_type<Self> const& from,
                                        cursor_type<Self> const& to)
    {
        if constexpr (N < End) {
            if (N < from.index()) {
                return distance_impl<N+1>(self, from, to);
            }

            assert(N == from.index());
            if (N == to.index()) {
                return flux::distance(std::get<N>(self.bases_),
                                      std::get<N>(from), std::get<N>(to));
            } else {
                auto dist_to_end = flux::distance(std::get<N>(self.bases_),
                                                  std::get<N>(from),
                                                  flux::last(std::get<N>(self.bases_)));
                auto remaining = distance_impl<N+1>(self, first_impl<N+1>(self), to);
                return dist_to_end + remaining;
            }
        } else {
            assert(N == from.index() && N == to.index());
            return flux::distance(std::get<N>(self.bases_), std::get<N>(from), std::get<N>(to));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto inc_ra_impl(Self& self, cursor_type<Self>& cur,
                                      distance_type offset)
        -> cursor_type<Self>&
    {
        if constexpr (N < End) {
            if (N < cur.index()) {
                return inc_ra_impl<N+1>(self, cur, offset);
            }

            assert(cur.index() == N);
            auto& base = std::get<N>(self.bases_);
            auto& base_cur = std::get<N>(cur);
            auto dist = flux::distance(base, base_cur, flux::last(base));
            if (offset < dist) {
                flux::inc(base, base_cur, offset);
                return cur;
            } else {
                cur = first_impl<N+1>(self);
                offset -= dist;
                return inc_ra_impl<N+1>(self, cur, offset);
            }
        } else {
            assert(cur.index() == N);
            flux::inc(std::get<N>(self.bases_), std::get<N>(cur), offset);
            return cur;
        }
    }

public:
    template <typename Self>
    static constexpr auto first(Self& self) -> cursor_type<Self>
    {
        return first_impl<0>(self);
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, cursor_type<Self> const& cur)
    {
        return cur.index() == End &&
               flux::is_last(std::get<End>(self.bases_), std::get<End>(cur));
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_type<Self> const& cur)
        -> std::common_reference_t<element_t<const_like_t<Self, Bases>>...>
    {
        return read_impl<0>(self, cur);
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_type<Self> const& cur)
        -> std::common_reference_t<rvalue_element_t<const_like_t<Self, Bases>>...>
    {
        return move_impl<0>(self, cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_type<Self>& cur)
        -> cursor_type<Self>&
    {
        return inc_impl<0>(self, cur);
    }

    template <typename Self>
    static constexpr auto dec(Self& self, cursor_type<Self>& cur)
        -> cursor_type<Self>&
        requires (bidirectional_sequence<const_like_t<Self, Bases>> && ...) &&
                 (bounded_sequence<const_like_t<Self,Bases>> &&...)
    {
        return dec_impl<sizeof...(Bases) - 1>(self, cur);
    }

    template <typename Self>
    static constexpr auto last(Self& self) -> cursor_type<Self>
        requires bounded_sequence<decltype(std::get<End>(self.bases_))>
    {
        constexpr auto Last = sizeof...(Bases) - 1;
        return cursor_type<Self>(std::in_place_index<Last>, flux::last(std::get<Last>(self.bases_)));
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        requires (sized_sequence<const_like_t<Self, Bases>> && ...)
    {
        return std::apply([](auto&... bases) { return (flux::size(bases) + ...); },
                          self.bases_);
    }

    template <typename Self>
    static constexpr auto for_each_while(Self& self, auto&& pred)
    {
        return for_each_while_impl<0>(self, pred);
    }

    template <typename Self>
    static constexpr auto distance(Self& self, cursor_type<Self> const& from,
                                   cursor_type<Self> const& to)
        -> distance_type
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...) &&
                 (bounded_sequence<const_like_t<Self, Bases>> && ...)
    {
        if (from.index() <= to.index()) {
            return distance_impl<0>(self, from, to);
        } else {
            return -distance_impl<0>(self, to, from);
        }
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_type<Self>& cur, distance_type offset)
        -> cursor_type<Self>&
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...) &&
                 (bounded_sequence<const_like_t<Self, Bases>> && ...)
    {
        return inc_ra_impl<0>(self, cur, offset);
    }

};

inline constexpr auto chain = detail::chain_fn{};

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_COUNT_HPP_INCLUDED
#define FLUX_OP_COUNT_HPP_INCLUDED



namespace flux {

namespace detail {

struct count_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> distance_t<Seq>
    {
        if constexpr (sized_sequence<Seq>) {
            return flux::size(seq);
        } else {
            distance_t<Seq> counter = 0;
            flux::for_each_while(seq, [&](auto&&) {
                ++counter;
                return true;
            });
            return counter;
        }
    }

    template <sequence Seq, typename Value, typename Proj = std::identity>
        requires std::equality_comparable_with<projected_t<Proj, Seq>, Value const&>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Value const& value, Proj proj = {}) const
        -> distance_t<Seq>
    {
        distance_t<Seq> counter = 0;
        flux::for_each_while(seq, [&](auto&& elem) {
            if (value == std::invoke(proj, FLUX_FWD(elem))) {
                ++counter;
            }
            return true;
        });
        return counter;
    }
};

struct count_if_fn {
    template <sequence Seq, typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Seq, Proj>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred, Proj proj = {}) const
        -> distance_t<Seq>
    {
        distance_t<Seq> counter = 0;
        flux::for_each_while(seq, [&](auto&& elem) {
            if (std::invoke(pred, std::invoke(proj, FLUX_FWD(elem)))) {
                ++counter;
            }
            return true;
        });
        return counter;
    }
};

} // namespace detail

inline constexpr auto count = detail::count_fn{};
inline constexpr auto count_if = detail::count_if_fn{};

template <typename D>
constexpr auto lens_base<D>::count()
{
    return flux::count(derived());
}

template <typename D>
template <typename Value, typename Proj>
    requires std::equality_comparable_with<projected_t<Proj, D>, Value const&>
constexpr auto lens_base<D>::count(Value const& value, Proj proj)
{
    return flux::count(derived(), value, std::move(proj));
}

template <typename D>
template <typename Pred, typename Proj>
    requires predicate_for<Pred, D, Proj>
constexpr auto lens_base<D>::count_if(Pred pred, Proj proj)
{
    return flux::count_if(derived(), std::move(pred), std::move(proj));
}

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_DROP_HPP_INCLUDED
#define FLUX_OP_DROP_HPP_INCLUDED




#include <optional>

namespace flux {

namespace detail {

template <lens Base>
struct drop_adaptor : lens_base<drop_adaptor<Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    distance_t<Base> count_;
    std::optional<cursor_t<Base>> cached_first_;

    friend struct sequence_iface<drop_adaptor>;

public:
    constexpr drop_adaptor(Base&& base, distance_t<Base> count)
        : base_(std::move(base)),
          count_(count)
    {}

    constexpr Base& base() & { return base_; }
    constexpr Base const& base() const& { return base_; }
};

struct drop_fn {
    template <adaptable_sequence Seq>
    constexpr auto operator()(Seq&& seq, distance_t<Seq> count) const
    {
        return drop_adaptor(flux::from(FLUX_FWD(seq)), count);
    }

};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::drop_adaptor<Base>>
    : detail::passthrough_iface_base<Base>
{
    using self_t = detail::drop_adaptor<Base>;

    using value_type = value_t<Base>;
    using distance_type = distance_t<Base>;

    static constexpr bool disable_multipass = !multipass_sequence<Base>;

    static constexpr auto first(self_t& self)
    {
        if constexpr (std::copy_constructible<cursor_t<Base>>) {
            if (!self.cached_first_) {
                self.cached_first_ = flux::next(self.base_, flux::first(self.base()), self.count_);
            }

            return *self.cached_first_;
        } else {
            return flux::next(self.base_, flux::first(self.base()), self.count_);
        }
    }

    static constexpr auto size(self_t& self)
        requires sized_sequence<Base>
    {
        return flux::size(self.base()) - self.count_;
    }

    static constexpr auto data(self_t& self)
        requires contiguous_sequence<Base>
    {
        return flux::data(self.base()) + self.count_;
    }

    void for_each_while(...) = delete;
};

inline constexpr auto drop = detail::drop_fn{};

template <typename Derived>
template <std::same_as<Derived> D>
constexpr auto lens_base<Derived>::drop(distance_t<D> count) &&
{
    return detail::drop_adaptor<Derived>(std::move(derived()), count);
}

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_DROP_WHILE_HPP_INCLUDED
#define FLUX_OP_DROP_WHILE_HPP_INCLUDED




#include <optional>

namespace flux {

namespace detail {

template <lens Base, typename Pred>
struct drop_while_adaptor : lens_base<drop_while_adaptor<Base, Pred>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;
    std::optional<cursor_t<Base>> cached_first_{};

    friend struct sequence_iface<drop_while_adaptor>;
    friend struct passthrough_iface_base<Base>;

    constexpr auto base() & -> Base& { return base_; }

public:
    constexpr drop_while_adaptor(Base&& base, Pred&& pred)
        : base_(std::move(base)),
          pred_(std::move(pred))
    {}
};

struct drop_while_fn {
    template <adaptable_sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr auto operator()(Seq&& seq, Pred pred) const
    {
        return drop_while_adaptor(flux::from(FLUX_FWD(seq)), std::move(pred));
    }
};

} // namespace detail

template <typename Base, typename Pred>
struct sequence_iface<detail::drop_while_adaptor<Base, Pred>>
    : detail::passthrough_iface_base<Base>
{
    using self_t = detail::drop_while_adaptor<Base, Pred>;

    static constexpr auto first(self_t& self)
    {
        if constexpr (std::copy_constructible<cursor_t<Base>>) {
            if (!self.cached_first_) {
                self.cached_first_ = flux::for_each_while(self.base_, self.pred_);
            }
            return *self.cached_first_;
        } else {
            return flux::for_each_while(self.base_, self.pred_);
        }
    }

    static constexpr auto data(self_t& self)
        requires contiguous_sequence<Base>
    {
        return flux::data(self.base_) +
                    flux::distance(self.base_, flux::first(self.base_), first(self));
    }

    void size(...) = delete;
    void for_each_while(...) = delete;
};

inline constexpr auto drop_while = detail::drop_while_fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto lens_base<D>::drop_while(Pred pred) &&
{
    return flux::drop_while(std::move(derived()), std::move(pred));
};

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FOR_EACH_HPP_INCLUDED
#define FLUX_OP_FOR_EACH_HPP_INCLUDED



namespace flux {

namespace detail {

struct for_each_fn {

    template <sequence Seq, typename Func, typename Proj = std::identity>
        requires std::invocable<Func&, projected_t<Proj, Seq>>
    constexpr auto operator()(Seq&& seq, Func func, Proj proj = {}) const -> Func
    {
        (void) flux::for_each_while(FLUX_FWD(seq), [&](auto&& elem) {
            std::invoke(func, std::invoke(proj, FLUX_FWD(elem)));
            return true;
        });
        return func;
    }
};

} // namespace detail

inline constexpr auto for_each = detail::for_each_fn{};

template <typename D>
template <typename Func, typename Proj>
    requires std::invocable<Func&, projected_t<Proj, D>>
constexpr auto lens_base<D>::for_each(Func func, Proj proj) -> Func
{
    return flux::for_each(derived(), std::move(func), std::move(proj));
}

} // namespace flux

#endif



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FILTER_HPP_INCLUDED
#define FLUX_OP_FILTER_HPP_INCLUDED





#include <optional>

namespace flux {

namespace detail {

template <lens Base, typename Pred>
    requires std::predicate<Pred&, element_t<Base>&>
class filter_adaptor : public lens_base<filter_adaptor<Base, Pred>>
{
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;
    std::optional<cursor_t<Base>> cached_first_{};

public:
    constexpr filter_adaptor(Base&& base, Pred&& pred)
        : base_(std::move(base)),
          pred_(std::move(pred))
    {}

    [[nodiscard]]
    constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]]
    constexpr auto base() && -> Base { return std::move(base_); }

    friend struct sequence_iface<filter_adaptor>;
};


struct filter_fn {
    template <adaptable_sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>&>
    constexpr auto operator()(Seq&& seq, Pred pred) const
    {
        return filter_adaptor(flux::from(FLUX_FWD(seq)), std::move(pred));
    }
};

} // namespace detail

template <typename Base, typename Pred>
struct sequence_iface<detail::filter_adaptor<Base, Pred>>
{
    using self_t = detail::filter_adaptor<Base, Pred>;

    static constexpr bool disable_multipass = !multipass_sequence<Base>;

    static constexpr auto first(self_t& self) -> cursor_t<Base>
    {
        // If the cursor_type is not copy-constructible, we can't cache it
        // and hand out copies. But this only applies to single-pass sequences
        // where we probably can't call first() more than once anyway
        if constexpr (!std::copy_constructible<cursor_t<Base>>) {
            return flux::for_each_while(self.base_, [&](auto&& elem) {
                return !std::invoke(self.pred_, elem);
            });
        } else {
            if (!self.cached_first_) {
                self.cached_first_ =
                    flux::for_each_while(self.base_, [&](auto&& elem) {
                        return !std::invoke(self.pred_, elem);
                    });
            }

            return *self.cached_first_;
        }
    }

    static constexpr auto is_last(self_t& self, cursor_t<Base> const& cur) -> bool
    {
        return flux::is_last(self.base_, cur);
    }

    static constexpr auto read_at(self_t& self, cursor_t<Base> const& cur)
        -> element_t<Base>
    {
        return flux::read_at(self.base_, cur);
    }

    static constexpr auto inc(self_t& self, cursor_t<Base>& cur) -> cursor_t<Base>&
    {
        // base_[{next(base_, cur), _}].for_each_while(!pred)?
        while (!flux::is_last(self.base_, flux::inc(self.base_, cur))) {
            if (std::invoke(self.pred_, flux::read_at(self.base_, cur))) {
                break;
            }
        }

        return cur;
    }

    static constexpr auto dec(self_t& self, cursor_t<Base>& cur) -> cursor_t<Base>&
        requires bidirectional_sequence<Base>
    {
        do {
            flux::dec(self.base_, cur);
        } while(!std::invoke(self.pred_, flux::read_at(self.base_, cur)));

        return cur;
    }

    static constexpr auto last(self_t& self) -> cursor_t<Base>
        requires bounded_sequence<Base>
    {
        return flux::last(self.base_);
    }

    static constexpr auto for_each_while(self_t& self, auto&& func) -> cursor_t<Base>
    {
        return flux::for_each_while(self.base_, [&](auto&& elem) {
            if (std::invoke(self.pred_, elem)) {
                return std::invoke(func, FLUX_FWD(elem));
            } else {
                return true;
            }
        });
    }
};

inline constexpr auto filter = detail::filter_fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>&>
constexpr auto lens_base<D>::filter(Pred pred) &&
{
    return detail::filter_adaptor<D, Pred>(std::move(derived()), std::move(pred));
}


} // namespace flux

#endif // FLUX_OP_FILTER_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FIND_HPP_INCLUDED
#define FLUX_OP_FIND_HPP_INCLUDED




namespace flux {

namespace detail {

struct find_fn {
    template <sequence Seq, typename Value, typename Proj = std::identity>
        requires std::equality_comparable_with<projected_t<Proj, Seq>, Value const&>
    constexpr auto operator()(Seq&& seq, Value const& value,
                              Proj proj = {}) const -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return std::invoke(proj, FLUX_FWD(elem)) != value;
        });
    }
};

struct find_if_fn {
    template <sequence Seq, typename Pred, typename Proj = std::identity>
        requires std::predicate<Pred&, projected_t<Proj, Seq>>
    constexpr auto operator()(Seq&& seq, Pred pred, Proj proj = {}) const
        -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, std::invoke(proj, FLUX_FWD(elem)));
        });
    }
};

struct find_if_not_fn {
    template <sequence Seq, typename Pred, typename Proj = std::identity>
        requires std::predicate<Pred&, projected_t<Proj, Seq>>
    constexpr auto operator()(Seq&& seq, Pred pred, Proj proj = {}) const
        -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return std::invoke(pred, std::invoke(proj, FLUX_FWD(elem)));
        });
    }
};

} // namespace detail

inline constexpr auto find = detail::find_fn{};
inline constexpr auto find_if = detail::find_if_fn{};
inline constexpr auto find_if_not = detail::find_if_not_fn{};

template <typename D>
template <typename Value, typename Proj>
    requires std::equality_comparable_with<projected_t<Proj, D>, Value const&>
constexpr auto lens_base<D>::find(Value const& val, Proj proj)
{
    return flux::find(derived(), val, std::ref(proj));
}

template <typename D>
template <typename Pred, typename Proj>
    requires predicate_for<Pred, D, Proj>
constexpr auto lens_base<D>::find_if(Pred pred, Proj proj)
{
    return flux::find_if(derived(), std::ref(pred), std::ref(proj));
}

template <typename D>
template <typename Pred, typename Proj>
    requires predicate_for<Pred, D, Proj>
constexpr auto lens_base<D>::find_if_not(Pred pred, Proj proj)
{
    return flux::find_if_not(derived(), std::ref(pred), std::ref(proj));
}

} // namespace flux

#endif // FLUX_OP_FIND_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_INPLACE_REVERSE_HPP_INCLUDED
#define FLUX_OP_INPLACE_REVERSE_HPP_INCLUDED



namespace flux {

namespace detail {

struct inplace_reverse_fn {
    template <bidirectional_sequence Seq>
        requires bounded_sequence<Seq> &&
                 element_swappable_with<Seq, Seq>
    constexpr void operator()(Seq&& seq) const
    {
        auto first = flux::first(seq);
        auto last = flux::last(seq);

        while (first != last && first != flux::dec(seq, last)) {
            flux::swap_at(seq, first, last);
            flux::inc(seq, first);
        }
    }
};

} // namespace detail

inline constexpr auto inplace_reverse = detail::inplace_reverse_fn{};

template <typename D>
constexpr auto lens_base<D>::inplace_reverse()
    requires bounded_sequence<D> && detail::element_swappable_with<D, D>
{
    return flux::inplace_reverse(derived());
}

} // namespace flux

#endif // FLUX_OP_INPLACE_REVERSE_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_MAP_HPP_INCLUDED
#define FLUX_OP_MAP_HPP_INCLUDED




namespace flux {

namespace detail {

template <lens Base, typename Func>
    requires std::is_object_v<Func> &&
             std::regular_invocable<Func&, element_t<Base>>
struct map_adaptor : lens_base<map_adaptor<Base, Func>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

    friend struct sequence_iface<map_adaptor>;

public:
    constexpr map_adaptor(Base&& base, Func&& func)
        : base_(std::move(base)),
          func_(std::move(func))
    {}

    constexpr auto base() & -> Base& { return base_; }
    constexpr auto base() const& -> Base const& { return base_; }
    constexpr auto base() && -> Base&& { return std::move(base_); }
    constexpr auto base() const&& -> Base const&& { return std::move(base_); }
};

struct map_fn {

    template <adaptable_sequence Seq, typename Func>
        requires std::regular_invocable<Func&, element_t<Seq>>
    constexpr auto operator()(Seq&& seq, Func func) const
    {
        return map_adaptor(flux::from(FLUX_FWD(seq)), std::move(func));
    }
};

} // namespace detail

template <typename Base, typename Func>
struct sequence_iface<detail::map_adaptor<Base, Func>>
    : detail::passthrough_iface_base<Base>
{
    using value_type = std::remove_cvref_t<std::invoke_result_t<Func&, element_t<Base>>>;

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(std::invoke(self.func_, flux::read_at(self.base_, cur)))
    {
        return std::invoke(self.func_, flux::read_at(self.base_, cur));
    }

    static constexpr auto for_each_while(auto& self, auto&& pred)
    {
        return flux::for_each_while(self.base_, [&](auto&& elem) {
            return std::invoke(pred, std::invoke(self.func_, FLUX_FWD(elem)));
        });
    }

    static void move_at() = delete; // Use the base version of move_at
    static void data() = delete; // we're not a contiguous sequence
};

inline constexpr auto map = detail::map_fn{};

template <typename Derived>
template <typename Func>
    requires std::invocable<Func&, element_t<Derived>>
constexpr auto lens_base<Derived>::map(Func func) &&
{
    return detail::map_adaptor<Derived, Func>(std::move(derived()), std::move(func));
}

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_REVERSE_HPP_INCLUDED
#define FLUX_OP_REVERSE_HPP_INCLUDED




namespace flux {

namespace detail {

template <typename BaseCur>
struct rev_cur {
    BaseCur base_cur;

    friend bool operator==(rev_cur const&, rev_cur const&)
        requires std::equality_comparable<BaseCur>
        = default;

    friend std::strong_ordering operator<=>(rev_cur const& lhs, rev_cur const& rhs)
        requires std::three_way_comparable<BaseCur>
    {
        return rhs <=> lhs;
    }
};

template <typename B>
rev_cur(B&&) -> rev_cur<std::remove_cvref_t<B>>;

template <lens Base>
    requires bidirectional_sequence<Base> &&
             bounded_sequence<Base>
struct reverse_adaptor : lens_base<reverse_adaptor<Base>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;

    friend struct sequence_iface<reverse_adaptor>;

public:
    constexpr explicit reverse_adaptor(Base&& base)
        : base_(std::move(base))
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base&& { return std::move(base_); }
};

template <typename>
inline constexpr bool is_reverse_adaptor = false;

template <typename Base>
inline constexpr bool is_reverse_adaptor<reverse_adaptor<Base>> = true;

struct reverse_fn {
    template <adaptable_sequence Seq>
        requires bidirectional_sequence<Seq> &&
                 bounded_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
        -> lens auto
    {
        if constexpr (is_reverse_adaptor<std::decay_t<Seq>>) {
            return FLUX_FWD(seq).base();
        } else {
            return reverse_adaptor(flux::from(FLUX_FWD(seq)));
        }
    }
};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::reverse_adaptor<Base>>
{
    using value_type = value_t<Base>;
    using distance_type = distance_t<Base>;

    static constexpr auto first(auto& self)
    {
        return detail::rev_cur(flux::last(self.base_));
    }

    static constexpr auto last(auto& self)
    {
        return detail::rev_cur(flux::first(self.base_));
    }

    static constexpr auto is_last(auto& self, auto const& cur) -> bool
    {
        return cur.base_cur == flux::first(self.base_);
    }

    static constexpr auto read_at(auto& self, auto const& cur) -> decltype(auto)
    {
        return flux::read_at(self.base_, flux::prev(self.base_, cur.base_cur));
    }

    static constexpr auto inc(auto& self, auto& cur) -> auto&
    {
        flux::dec(self.base_, cur.base_cur);
        return cur;
    }

    static constexpr auto dec(auto& self, auto& cur) -> auto&
    {
        flux::inc(self.base_, cur.base_cur);
        return cur;
    }

    static constexpr auto inc(auto& self, auto& cur, auto dist) -> auto&
        requires random_access_sequence<decltype(self.base_)>
    {
        flux::inc(self.base_, cur.base_cur, -dist);
        return cur;
    }

    static constexpr auto distance(auto& self, auto const& from, auto const& to)
        requires random_access_sequence<decltype(self.base_)>
    {
        return -flux::distance(self.base_, from.base_cur, to.base_cur);
    }

    // FIXME: GCC11 ICE
#if GCC_ICE
    static constexpr auto size(auto& self)
        requires sized_sequence<decltype(self.base_)>
    {
        return flux::size(self.base_);
    }
#endif

    static constexpr auto move_at(auto& self, auto const& cur) -> decltype(auto)
    {
        return flux::move_at(self.base_, cur.base_cur);
    }

    static constexpr auto for_each_while(auto& self, auto&& pred)
    {
        auto cur = flux::last(self.base_);
        const auto end = flux::first(self.base_);

        while (cur != end) {
            if (!std::invoke(pred, flux::read_at(self.base_, flux::prev(self.base_, cur)))) {
                break;
            }
            flux::dec(self.base_, cur);
        }

        return detail::rev_cur(cur);
    }
};

inline constexpr auto reverse = detail::reverse_fn{};

template <typename D>
constexpr auto lens_base<D>::reverse() &&
    requires bidirectional_sequence<D> && bounded_sequence<D>
{
    return flux::reverse(std::move(derived()));
}

} // namespace flux

#endif



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SPLIT_HPP_INCLUDED
#define FLUX_OP_SPLIT_HPP_INCLUDED





// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once



namespace flux {

namespace detail {

struct search_fn {
    template <multipass_sequence Haystack, multipass_sequence Needle>
        // Requires...
    constexpr auto operator()(Haystack&& h, Needle&& n) const
        -> bounds_t<Haystack>
    {
        auto hfirst = flux::first(h);

        while(true) {
            auto cur1 = hfirst;
            auto cur2 = flux::first(n);

            while (true) {
                if (is_last(n, cur2)) {
                    return {std::move(hfirst), std::move(cur1)};
                }

                if (is_last(h, cur1)) {
                    return {cur1, cur1};
                }

                if (read_at(h, cur1) != read_at(n, cur2)) {
                    break;
                }

                inc(h, cur1);
                inc(n, cur2);
            }

            inc(h, hfirst);
        }
    }

};

} // namespace detail

inline constexpr auto search = detail::search_fn{};

} // namespace flux



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_SINGLE_HPP_INCLUDED
#define FLUX_SOURCE_SINGLE_HPP_INCLUDED



#include <cassert>

namespace flux {

namespace detail {

template <std::movable T>
struct single_sequence : lens_base<single_sequence<T>> {
private:
    T obj_;

    friend struct sequence_iface<single_sequence>;

public:
    constexpr single_sequence()
        requires std::default_initializable<T>
    = default;

    constexpr explicit single_sequence(T const& obj)
        requires std::copy_constructible<T>
    : obj_(obj)
    {}

    constexpr explicit single_sequence(T&& obj)
        requires std::move_constructible<T>
    : obj_(std::move(obj))
    {}

    template <typename... Args>
    constexpr explicit single_sequence(std::in_place_t, Args&&... args)
        requires std::constructible_from<T, Args...>
    : obj_(FLUX_FWD(args)...)
    {}

    constexpr auto value() -> T& { return obj_; }
    constexpr auto value() const -> T const& { return obj_; }
};

struct single_fn {
    template <typename T>
    constexpr auto operator()(T&& t) const -> single_sequence<std::decay_t<T>>
    {
        return single_sequence<std::decay_t<T>>(FLUX_FWD(t));
    }
};

} // namespace detail

template <typename T>
struct sequence_iface<detail::single_sequence<T>>
{
private:
    using self_t = detail::single_sequence<T>;

    enum class cursor_type : bool { valid, done };

public:

    static constexpr auto first(self_t const&) { return cursor_type::valid; }

    static constexpr auto last(self_t const&) { return cursor_type::done; }

    static constexpr bool is_last(self_t const&, cursor_type cur)
    {
        return cur == cursor_type::done;
    }

    static constexpr auto read_at(auto& self, cursor_type cur) -> auto&
    {
        assert(cur == cursor_type::valid);
        return self.obj_;
    }

    static constexpr auto inc(self_t const&, cursor_type& cur) -> cursor_type&
    {
        assert(cur == cursor_type::valid);
        cur = cursor_type::done;
        return cur;
    }

    static constexpr auto dec(self_t const&, cursor_type& cur) -> cursor_type&
    {
        assert(cur == cursor_type::done);
        cur = cursor_type::valid;
        return cur;
    }

    static constexpr auto inc(self_t const&, cursor_type& cur, std::ptrdiff_t off)
        -> cursor_type&
    {
        if (off > 0) {
            assert(cur == cursor_type::valid && off == 1);
            cur = cursor_type::done;
        } else if (off < 0) {
            assert(cur == cursor_type::done && off == -1);
            cur = cursor_type::valid;
        }
        return cur;
    }

    static constexpr auto distance(self_t const&, cursor_type from,
                                   cursor_type to)
        -> std::ptrdiff_t
    {
        return static_cast<int>(to) - static_cast<int>(from);
    }

    static constexpr auto size(self_t const&) { return 1; }

    static constexpr auto data(auto& self)
    {
        return std::addressof(self.obj_);
    }

    static constexpr auto for_each_while(auto& self, auto&& pred)
    {
        return std::invoke(pred, self.obj_) ? cursor_type::done : cursor_type::valid;
    }

};

inline constexpr auto single = detail::single_fn{};

} // namespace flux

#endif // FLUX_SOURCE_SINGLE_HPP_INCLUDED


namespace flux {

namespace detail {

template <lens Base, lens Pattern>
struct split_adaptor : lens_base<split_adaptor<Base, Pattern>> {
private:
    Base base_;
    Pattern pattern_;

    friend struct sequence_iface<split_adaptor>;

public:
    constexpr split_adaptor(Base&& base, Pattern&& pattern)
        : base_(std::move(base)),
          pattern_(std::move(pattern))
    {}
};

struct split_fn {
    template <adaptable_sequence Seq, adaptable_sequence Pattern>
        requires multipass_sequence<Seq> &&
                 multipass_sequence<Pattern> &&
                 std::equality_comparable_with<element_t<Seq>, element_t<Pattern>>
    constexpr auto operator()(Seq&& seq, Pattern&& pattern) const
    {
        return split_adaptor(flux::from(FLUX_FWD(seq)), flux::from(FLUX_FWD(pattern)));
    }

    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    constexpr auto operator()(Seq&& seq, value_t<Seq> delim) const
    {
        return (*this)(FLUX_FWD(seq), flux::single(std::move(delim)));
    }
};

template <typename From, typename To>
using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

template <typename>
inline constexpr bool is_single_seq = false;

template <typename T>
inline constexpr bool is_single_seq<single_sequence<T>> = true;

} // namespace detail

template <typename Base, typename Pattern>
struct sequence_iface<detail::split_adaptor<Base, Pattern>>
{
private:
    template <typename Self, typename B = detail::const_like_t<Self, Base>>
    struct cursor_type {
        cursor_t<B> cur;
        bounds_t<B> next;
        bool trailing_empty = false;

        friend bool operator==(cursor_type const&,
                               cursor_type const&) = default;
    };

    static constexpr auto find_next(auto& self, auto const& from)
    {
        if constexpr (detail::is_single_seq<decltype(self.pattern_)>) {
            // auto cur = self.base_[{cur, last}].find(self.pattern_.value());
            auto cur = flux::find(flux::slice(self.base_, from, flux::last),
                                  self.pattern_.value());
            if (flux::is_last(self.base_, cur)) {
                return bounds{cur, cur};
            } else {
                return bounds{cur, flux::next(self.base_, cur)};
            }
        } else {
            return flux::search(flux::slice(self.base_, from, flux::last),
                                self.pattern_);
        }
    }

public:

    static constexpr bool is_infinite = infinite_sequence<Base>;

    template <typename Self>
    static constexpr auto first(Self& self)
        requires sequence<decltype(self.base_)>
    {
        auto bounds = flux::search(self.base_, self.pattern_);
        return cursor_type<Self>(flux::first(self.base_), std::move(bounds));
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, cursor_type<Self> const& cur)
        requires sequence<decltype(self.base_)>
    {
        return flux::is_last(self.base_, cur.cur) && !cur.trailing_empty;
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_type<Self> const& cur)
        requires sequence<decltype(self.base_)>
    {
        return flux::slice(self.base_, cur.cur, cur.next.from);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_type<Self>& cur) -> cursor_type<Self>&
        requires sequence<decltype(self.base_)>
    {
        cur.cur = cur.next.from;
        if (!flux::is_last(self.base_, cur.cur)) {
            cur.cur = cur.next.to;
            if (flux::is_last(self.base_, cur.cur)) {
                cur.trailing_empty = true;
                cur.next = {cur.cur, cur.cur};
            } else {
                cur.next = find_next(self, cur.cur);
            }
        } else {
            cur.trailing_empty = false;
        }
        return cur;
    }
};

inline constexpr auto split = detail::split_fn{};

template <typename Derived>
template <multipass_sequence Pattern>
    requires std::equality_comparable_with<element_t<Derived>, element_t<Pattern>>
constexpr auto lens_base<Derived>::split(Pattern&& pattern) &&
{
    return flux::split(std::move(derived()), FLUX_FWD(pattern));
}

template <typename Derived>
template <typename ValueType>
    requires decays_to<ValueType, value_t<Derived>>
constexpr auto lens_base<Derived>::split(ValueType&& delim) &&
{
    return flux::split(std::move(derived()), FLUX_FWD(delim));
}


} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_STRING_SPLIT_HPP_INCLUDED
#define FLUX_STRING_SPLIT_HPP_INCLUDED



#include <string_view>

namespace flux {

namespace detail {

template <typename T, typename... U>
concept any_of = (std::same_as<T, U> || ...);

template <typename C>
concept character = any_of<C, char, wchar_t, char8_t, char16_t, char32_t>;

struct to_string_view_fn {
    template <contiguous_sequence Seq>
        requires sized_sequence<Seq> && character<value_t<Seq>>
    constexpr auto operator()(Seq&& seq) const
    {
        return std::basic_string_view<value_t<Seq>>(flux::data(seq), flux::size(seq));
    }
};

inline constexpr auto to_string_view = to_string_view_fn{};

struct split_string_fn {

    template <contiguous_sequence Seq, multipass_sequence Pattern>
        requires character<value_t<Seq>> &&
                std::equality_comparable_with<element_t<Seq>, element_t<Pattern>>
    constexpr auto operator()(Seq&& seq, Pattern&& pattern) const
    {
        return flux::split(FLUX_FWD(seq), FLUX_FWD(pattern)).map(to_string_view);
    }

    // Attempt to hijack string literal patterns to do the right thing
    template <contiguous_sequence Seq, std::size_t N>
        requires character<value_t<Seq>>
    constexpr auto operator()(Seq&& seq, value_t<Seq> const (&pattern)[N]) const
    {
        return flux::split(FLUX_FWD(seq), std::basic_string_view(pattern))
                    .map(to_string_view);
    }

    template <contiguous_sequence Seq>
        requires character<value_t<Seq>>
    constexpr auto operator()(Seq&& seq, value_t<Seq> delim) const
    {
        return flux::split(FLUX_FWD(seq), delim).map(to_string_view);
    }
};

} // namespace detail

inline constexpr auto split_string = detail::split_string_fn{};

template <typename D>
constexpr auto lens_base<D>::split_string(auto&& pattern) &&
{
    return flux::split_string(std::move(derived()), FLUX_FWD(pattern));
}


} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SWAP_ELEMENTS_HPP_INCLUDED
#define FLUX_OP_SWAP_ELEMENTS_HPP_INCLUDED



namespace flux {

namespace detail {

struct swap_elements_fn {
    template <sequence Seq1, sequence Seq2>
        requires element_swappable_with<Seq1&, Seq2&>
    constexpr void operator()(Seq1&& seq1, Seq2&& seq2) const
    {
        auto cur1 = flux::first(seq1);
        auto cur2 = flux::first(seq2);

        while (!flux::is_last(seq1, cur1) && !flux::is_last(seq2, cur2)) {
            flux::swap_with(seq1, cur1, seq2, cur2);
            flux::inc(seq1, cur1);
            flux::inc(seq2, cur2);
        }
    }
};

}

inline constexpr auto swap_elements = detail::swap_elements_fn{};

}

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_TAKE_HPP_INCLUDED
#define FLUX_OP_TAKE_HPP_INCLUDED





namespace flux {

namespace detail {

template <typename Base>
struct take_adaptor : lens_base<take_adaptor<Base>>
{
private:
    Base base_;
    distance_t<Base> count_;

    template <bool IsConst>
    struct cursor_type {
    private:
        using base_t = std::conditional_t<IsConst, Base const, Base>;

    public:
        cursor_t<base_t> base_cur;
        distance_t<base_t> length;

        friend bool operator==(cursor_type const&, cursor_type const&) = default;
        friend auto operator<=>(cursor_type const& lhs, cursor_type const& rhs) = default;
    };

    friend struct sequence_iface<take_adaptor>;

public:
    constexpr take_adaptor(Base&& base, distance_t<Base> count)
        : base_(std::move(base)),
          count_(count)
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base&& { return std::move(base_); }
};

struct take_fn {

    template <adaptable_sequence Seq>
    constexpr auto operator()(Seq&& seq, distance_t<Seq> count) const
    {
        if constexpr (random_access_sequence<Seq> && std::is_lvalue_reference_v<Seq>) {
            auto first = flux::first(seq);
            auto last = flux::next(seq, first, count);
            return flux::slice(FLUX_FWD(seq), std::move(first), std::move(last));
        } else {
            return take_adaptor(flux::from(FLUX_FWD(seq)), count);
        }
    }
};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::take_adaptor<Base>> {

    template <typename Self>
    using cursor_t =
        typename std::remove_const_t<Self>::template cursor_type<std::is_const_v<Self>>;

    using value_type = value_t<Base>;
    using distance_type = distance_t<Base>;

    static constexpr bool disable_multipass = !multipass_sequence<Base>;

    template <typename Self>
    static constexpr auto first(Self& self)
    {
        return cursor_t<Self>{
            .base_cur = flux::first(self.base_),
            .length = self.count_
        };
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, cursor_t<Self> const& cur) -> bool
    {
        return cur.length <= 0 || flux::is_last(self.base_, cur.base_cur);
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
    {
        return flux::read_at(self.base_, cur.base_cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur) -> cursor_t<Self>&
    {
        flux::inc(self.base_, cur.base_cur);
        --cur.length;
        return cur;
    }

    template <typename Self>
    static constexpr auto dec(Self& self, cursor_t<Self>& cur) -> cursor_t<Self>&
        requires bidirectional_sequence<Base>
    {
        flux::dec(self.base_, cur.base_cur);
        ++cur.length;
        return cur;
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur, distance_t<Base> offset)
        -> cursor_t<Self>&
        requires random_access_sequence<Base>
    {
        flux::inc(self.base_, cur.base_cur, offset);
        cur.length -= offset;
        return cur;
    }

    template <typename Self>
    static constexpr auto distance(Self& self, cursor_t<Self> const& from, cursor_t<Self> const& to)
        requires random_access_sequence<Base>
    {
        return std::min(flux::distance(self.base_, from.base_cur, to.base_cur),
                        to.length - from.length);
    }

    template <typename Self>
    static constexpr auto data(Self& self)
    {
        return flux::data(self.base_);
    }

    template <typename Self>
    static constexpr auto last(Self& self)
        requires random_access_sequence<Base> && sized_sequence<Base>
    {
        return cursor_t<Self>{
            .base_cur = flux::next(self.base_, flux::first(self.base_), size(self)),
            .length = 0
        };
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        requires sized_sequence<Base>
    {
        return std::min(flux::size(self.base_), self.count_);
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
    {
        return flux::move_at(self.base_, cur.base_cur);
    }

    template <typename Self>
    static constexpr auto for_each_while(Self& self, auto&& pred)
    {
        auto count = self.count_;
        auto cur = flux::for_each_while(self.base_, [&pred, &count](auto&& elem) {
            if (count > 0) {
                --count;
                return std::invoke(pred, FLUX_FWD(elem));
            } else {
                return false;
            }
        });
        return cursor_t<Self>{.base_cur = std::move(cur), .length = count};
    }
};

inline constexpr auto take = detail::take_fn{};

template <typename Derived>
template <std::same_as<Derived> D>
constexpr auto lens_base<Derived>::take(distance_t<D> count) &&
{
    return detail::take_adaptor<D>(std::move(derived()), count);
}

} // namespace flux

#endif // FLUX_OP_TAKE_HPP_INCLUDED

// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_TAKE_WHILE_HPP_INCLUDED
#define FLUX_OP_TAKE_WHILE_HPP_INCLUDED




namespace flux {

namespace detail {

template <lens Base, typename Pred>
struct take_while_adaptor : lens_base<take_while_adaptor<Base, Pred>> {
private:
    Base base_;
    Pred pred_;

    constexpr auto base() & -> Base& { return base_; }

    friend struct sequence_iface<take_while_adaptor>;
    friend struct passthrough_iface_base<Base>;

public:
    constexpr take_while_adaptor(Base&& base, Pred&& pred)
        : base_(std::move(base)),
          pred_(std::move(pred))
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base { return std::move(base_); }
};

struct take_while_fn {
    template <adaptable_sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>&>
    constexpr auto operator()(Seq&& seq, Pred pred) const
    {
        return take_while_adaptor(flux::from(seq), std::move(pred));
    }
};

} // namespace detail

template <typename Base, typename Pred>
struct sequence_iface<detail::take_while_adaptor<Base, Pred>>
    : detail::passthrough_iface_base<Base>
{
    using self_t = detail::take_while_adaptor<Base, Pred>;

    static constexpr bool is_infinite = false;

    static constexpr bool is_last(self_t& self, cursor_t<Base> const& idx)
    {
        if (flux::is_last(self.base_, idx) ||
            !std::invoke(self.pred_, flux::read_at(self.base_, idx))) {
            return true;
        } else {
            return false;
        }
    }

    static constexpr bool is_last(self_t const& self, cursor_t<Base const> const& cur)
        requires std::predicate<Pred const&, element_t<Base const>>
    {
        if (flux::is_last(self.base_, cur) ||
            !std::invoke(self.pred_, flux::read_at(self.base_, cur))) {
            return true;
        } else {
            return false;
        }
    }

    void last() = delete;
    void size() = delete;

    static constexpr auto for_each_while(auto& self, auto&& func)
    {
        return flux::for_each_while(self.base_, [&](auto&& elem) {
            if (!std::invoke(self.pred_, elem)) {
                return false;
            } else {
                return std::invoke(func, FLUX_FWD(elem));
            }
        });
    }
};

inline constexpr auto take_while = detail::take_while_fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto lens_base<D>::take_while(Pred pred) &&
{
    return flux::take_while(std::move(derived()), std::move(pred));
}

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ZIP_HPP_INCLUDED
#define FLUX_OP_ZIP_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_EMPTY_HPP_INCLUDED
#define FLUX_SOURCE_EMPTY_HPP_INCLUDED



#include <cassert>

namespace flux {

namespace detail {

template <typename T>
    requires std::is_object_v<T>
struct empty_sequence {
private:
    struct cursor_type {
        friend constexpr bool operator==(cursor_type, cursor_type) = default;
        friend constexpr auto operator<=>(cursor_type, cursor_type) = default;
    };

public:
    static constexpr auto first() -> cursor_type { return {}; }
    static constexpr auto last() -> cursor_type { return {}; }
    static constexpr auto is_last(cursor_type) -> bool { return true; }
    static constexpr auto inc(cursor_type& cur, std::ptrdiff_t = 0) -> cursor_type& { return cur; }
    static constexpr auto dec(cursor_type& cur) -> cursor_type& { return cur; }
    static constexpr auto distance(cursor_type, cursor_type) -> std::ptrdiff_t { return 0; }
    static constexpr auto size() -> std::ptrdiff_t { return 0; }
    static constexpr auto data() -> T* { return nullptr; }

    static constexpr auto read_at(cursor_type) -> T&
    {
        assert(false && "Attempted read of flux::empty");
        /* Guaranteed UB... */
        return *static_cast<T*>(nullptr);
    }
};

} // namespace detail

template <typename T>
    requires std::is_object_v<T>
inline constexpr auto empty = detail::empty_sequence<T>{};

} // namespace flux

#endif // FLUX_SOURCE_EMPTY_HPP_INCLUDED


namespace flux {

namespace detail {

template <typename... Ts>
struct pair_or_tuple {
    using type = std::tuple<Ts...>;
};

template <typename T, typename U>
struct pair_or_tuple<T, U> {
    using type = std::pair<T, U>;
};

template <typename... Ts>
using pair_or_tuple_t = typename pair_or_tuple<Ts...>::type;

template <lens... Bases>
struct zip_adaptor : lens_base<zip_adaptor<Bases...>> {
private:
    pair_or_tuple_t<Bases...> bases_;

    friend struct sequence_iface<zip_adaptor>;

public:
    constexpr explicit zip_adaptor(Bases&&... bases)
        : bases_(std::move(bases)...)
    {}
};

struct zip_fn {
    template <adaptable_sequence... Seqs>
    constexpr auto operator()(Seqs&&... seqs) const
    {
        if constexpr (sizeof...(Seqs) == 0) {
            return empty<std::tuple<>>;
        } else {
            return zip_adaptor(flux::from(FLUX_FWD(seqs))...);
        }
    }
};

} // namespace detail

template <typename... Bases>
struct sequence_iface<detail::zip_adaptor<Bases...>>
{
private:
    template <typename... Ts>
    using tuple_t = detail::pair_or_tuple_t<Ts...>;

    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    template <std::size_t I>
    static constexpr decltype(auto) read_at_(auto& self, auto const& cur)
    {
        return flux::read_at(std::get<I>(self.bases_), std::get<I>(cur));
    }

    template <std::size_t I>
    static constexpr decltype(auto) move_at_(auto& self, auto const& cur)
    {
        return flux::move_at(std::get<I>(self.bases_), std::get<I>(cur));
    }

public:
    using value_type = tuple_t<value_t<Bases>...>;
    using distance_type = std::common_type_t<distance_t<Bases>...>;

    static constexpr bool is_infinite = (infinite_sequence<Bases> && ...);

    static constexpr auto first(auto& self)
    {
        return std::apply([](auto&&... args) {
            return tuple_t<decltype(flux::first(FLUX_FWD(args)))...>(flux::first(FLUX_FWD(args))...);
        }, self.bases_);
    }

    template <typename Self>
    static constexpr bool is_last(Self& self, cursor_t<Self> const& cur)
    {
        return [&self, &cur]<std::size_t... I>(std::index_sequence<I...>) {
            return (flux::is_last(std::get<I>(self.bases_), std::get<I>(cur)) || ...);
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return tuple_t<decltype(read_at_<I>(self, cur))...> {
                read_at_<I>(self, cur)...
            };
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto& inc(Self& self, cursor_t<Self>& cur)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::inc(std::get<I>(self.bases_), std::get<I>(cur)), ...);
        }(std::index_sequence_for<Bases...>{});

        return cur;
    }

    template <typename Self>
    static constexpr auto& dec(Self& self, cursor_t<Self>& cur)
        requires (bidirectional_sequence<const_like_t<Self, Bases>> && ...)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::dec(std::get<I>(self.bases_), std::get<I>(cur)), ...);
        }(std::index_sequence_for<Bases...>{});

        return cur;
    }

    template <typename Self>
    static constexpr auto& inc(Self& self, cursor_t<Self>& cur, distance_t<Self> offset)
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::inc(std::get<I>(self.bases_), std::get<I>(cur), offset), ...);
        }(std::index_sequence_for<Bases...>{});

        return cur;
    }

    template <typename Self>
    static constexpr auto distance(Self& self, cursor_t<Self> const& from,
                                   cursor_t<Self> const& to)
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return std::min({
                static_cast<distance_t<Self>>(
                    flux::distance(std::get<I>(self.bases_), std::get<I>(from), std::get<I>(to)))...});
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto last(Self& self)
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
            && (sized_sequence<const_like_t<Self, Bases>> && ...)
    {
        auto cur = first(self);
        return inc(self, cur, size(self));
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        requires (sized_sequence<const_like_t<Self, Bases>> && ...)
    {
        return std::apply([&](auto&... args) {
            return std::min({static_cast<distance_t<Self>>(flux::size(args))...});
        }, self.bases_);
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return tuple_t<decltype(move_at_<I>(self, cur))...>{
                move_at_<I>(self, cur)...
            };
        }(std::index_sequence_for<Bases...>{});
    }
};

inline constexpr auto zip = detail::zip_fn{};

} // namespace flux

#endif // FLUX_OP_ZIP_HPP_INCLUDED




// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_FROM_ISTREAM_HPP_INCLUDED
#define FLUX_SOURCE_FROM_ISTREAM_HPP_INCLUDED



#include <iosfwd>

namespace flux {

namespace detail {

template <typename T, typename CharT, typename Traits>
    requires std::default_initializable<T>
class istream_adaptor : public lens_base<istream_adaptor<T, CharT, Traits>> {
    using istream_type = std::basic_istream<CharT, Traits>;
    istream_type* is_ = nullptr;
    T val_ = T();

    friend struct sequence_iface<istream_adaptor>;

public:
    explicit istream_adaptor(istream_type& is)
        : is_(std::addressof(is))
    {}

};

template <std::default_initializable T>
struct from_istream_fn {

    template <typename CharT, typename Traits>
    [[nodiscard]]
    auto operator()(std::basic_istream<CharT, Traits>& is) const
    {
        return istream_adaptor<T, CharT, Traits>(is);
    }

};

} // namespace detail

template <typename T, typename CharT, typename Traits>
struct sequence_iface<detail::istream_adaptor<T, CharT, Traits>>
{
private:
    struct cursor_type {
        cursor_type(cursor_type&&) = default;
        cursor_type& operator=(cursor_type&&) = default;
    private:
        friend struct sequence_iface;
        explicit cursor_type() = default;
    };

    using self_t = detail::istream_adaptor<T, CharT, Traits>;

public:
    static auto first(self_t& self) -> cursor_type
    {
        cursor_type cur{};
        inc(self, cur);
        return cur;
    }

    static auto is_last(self_t& self, cursor_type const&) -> bool
    {
        return !(self.is_ && static_cast<bool>(*self.is_));
    }

    static auto read_at(self_t& self, cursor_type const&) -> T const&
    {
        return self.val_;
    }

    static auto inc(self_t& self, cursor_type& cur) -> cursor_type&
    {
        if (!(self.is_ && (*self.is_ >> self.val_))) {
            self.is_ = nullptr;
        }
        return cur;
    }
};

template <std::default_initializable T>
inline constexpr auto from_istream = detail::from_istream_fn<T>{};

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_ISTREAMBUF_HPP_INCLUDED
#define FLUX_SOURCE_ISTREAMBUF_HPP_INCLUDED





#include <iosfwd>

namespace flux {

namespace detail {

template <typename CharT, typename Traits>
void derives_from_streambuf_test(std::basic_streambuf<CharT, Traits>&);

template <typename T>
concept derives_from_streambuf = requires (T& t) { derives_from_streambuf_test(t); };

struct from_istreambuf_fn {
    template <typename CharT, typename Traits>
    [[nodiscard]]
    auto operator()(std::basic_streambuf<CharT, Traits>* streambuf) const -> lens auto
    {
        assert(streambuf != nullptr);
        return flux::from(*streambuf);
    }

    template <typename CharT, typename Traits>
    [[nodiscard]]
    auto operator()(std::basic_istream<CharT, Traits>& istream) const -> lens auto
    {
        return flux::from(*istream.rdbuf());
    }
};

} // namespace detail

template <detail::derives_from_streambuf Streambuf>
struct sequence_iface<Streambuf>
{
private:
    struct cursor_type {
        cursor_type(cursor_type&&) = default;
        cursor_type& operator=(cursor_type&&) = default;
    private:
        friend struct sequence_iface;
        cursor_type() = default;
    };

    using traits_type = typename Streambuf::traits_type;
    using char_type = typename Streambuf::char_type;

public:
    static auto first(Streambuf&) -> cursor_type { return {}; }

    static auto is_last(Streambuf& self, cursor_type const&) -> bool
    {
        return self.sgetc() == traits_type::eof();
    }

    static auto inc(Streambuf& self, cursor_type& cur) -> cursor_type&
    {
        self.sbumpc();
        return cur;
    }

    static auto read_at(Streambuf& self, cursor_type const&) -> char_type
    {
        return traits_type::to_char_type(self.sgetc());
    }
};

inline constexpr auto from_istreambuf = detail::from_istreambuf_fn{};

} // namespace flux

#endif




// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_RANGES_HPP_INCLUDED
#define FLUX_RANGES_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_RANGES_FROM_RANGE_HPP_INCLUDED
#define FLUX_RANGES_FROM_RANGE_HPP_INCLUDED



#include <ranges>

namespace flux {

template <typename R>
    requires (!detail::derived_from_lens_base<R> && std::ranges::input_range<R>)
struct sequence_iface<R> {

    using value_type = std::ranges::range_value_t<R>;
    using distance_type = std::ranges::range_difference_t<R>;

    static constexpr bool disable_multipass = !std::ranges::forward_range<R>;

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr auto first(Self& self)
    {
        return std::ranges::begin(self);
    }

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr bool is_last(Self& self, cursor_t<Self> const& cur)
    {
        return cur == std::ranges::end(self);
    }

    template <decays_to<R> Self>
    static constexpr auto read_at(Self&, cursor_t<Self> const& cur)
        -> decltype(auto)
    {
        return *cur;
    }

    template <decays_to<R> Self>
    static constexpr auto inc(Self&, cursor_t<Self>& cur)
        -> cursor_t<Self>&
    {
        return ++cur;
    }

    template <decays_to<R> Self>
        requires std::bidirectional_iterator<cursor_t<Self>>
    static constexpr auto dec(Self&, cursor_t<Self>& cur)
        -> cursor_t<Self>&
    {
        return --cur;
    }

    template <decays_to<R> Self>
        requires std::random_access_iterator<cursor_t<Self>>
    static constexpr auto inc(Self&, cursor_t<Self>& cur, distance_t<Self> offset)
        -> cursor_t<Self>&
    {
        return cur += offset;
    }

    template <decays_to<R> Self>
        requires std::random_access_iterator<cursor_t<Self>>
    static constexpr auto distance(Self&, cursor_t<Self> const& from,
                                   cursor_t<Self> const& to)
    {
        return to - from;
    }

    template <decays_to<R> Self>
        requires std::ranges::contiguous_range<Self>
    static constexpr auto data(Self& self)
    {
        return std::ranges::data(self);
    }

    template <decays_to<R> Self>
        requires std::ranges::common_range<Self>
    static constexpr auto last(Self& self) -> cursor_t<Self>
    {
        return std::ranges::end(self);
    }

    template <decays_to<R> Self>
        requires std::ranges::sized_range<Self>
    static constexpr auto size(Self& self)
    {
        return std::ranges::size(self);
    }

    template <decays_to<R> Self>
    static constexpr auto move_at(Self&, cursor_t<Self> const& cur)
        -> decltype(auto)
    {
        return std::ranges::iter_move(cur);
    }

    template <decays_to<R> Self>
    static constexpr auto slice(Self& self, cursor_t<Self> from)
    {
        return std::ranges::subrange(std::move(from), std::ranges::end(self));
    }

    template <decays_to<R> Self>
    static constexpr auto slice(Self&, cursor_t<Self> from, cursor_t<Self> to)
    {
        return std::ranges::subrange(std::move(from), std::move(to));
    }

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr auto for_each_while(Self& self, auto&& pred)
    {
        auto iter = std::ranges::begin(self);
        auto const last = std::ranges::end(self);

        for ( ; iter != last; ++iter) {
            if (!std::invoke(pred, *iter)) {
                break;
            }
        }

        return iter;
    }
};

} // namespace flux

#endif // FLUX_RANGE_FROM_RANGE_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_RANGES_VIEW_HPP_INCLUDED
#define FLUX_RANGES_VIEW_HPP_INCLUDED




#include <ranges>

namespace flux {

namespace detail {

template <sequence Base>
consteval auto get_iterator_tag()
{
    if constexpr (contiguous_sequence<Base>) {
        return std::contiguous_iterator_tag{};
    } else if constexpr (random_access_sequence<Base>) {
        return std::random_access_iterator_tag{};
    } else if constexpr (bidirectional_sequence<Base>) {
        return std::bidirectional_iterator_tag{};
    } else if constexpr (multipass_sequence<Base>) {
        return std::forward_iterator_tag{};
    } else {
        return std::input_iterator_tag{};
    }
}

template <lens Base>
struct view_adaptor : std::ranges::view_interface<view_adaptor<Base>>
{
private:
    [[no_unique_address]] Base base_;

    template <bool IsConst>
    class iterator {
        using S = std::conditional_t<IsConst, Base const, Base>;
        S* base_ = nullptr;
        cursor_t<S> cur_{};

    public:
        using value_type = value_t<S>;
        using difference_type = distance_t<S>;
        using element_type = value_t<S>;
        using iterator_concept = decltype(get_iterator_tag<S>());

        iterator() requires std::default_initializable<cursor_t<S>> = default;

        constexpr iterator(S& base, cursor_t<S> cur)
            : base_(std::addressof(base)),
              cur_(std::move(cur))
        {}

        constexpr iterator(iterator<!IsConst> other)
            requires IsConst && std::convertible_to<cursor_t<Base>,
                            cursor_t<S>>
            : base_(other.base_),
              cur_(std::move(other.cur_))
        {}

        constexpr auto operator*() const -> element_t<S>
        {
            return flux::read_at(*base_, cur_);
        }

        constexpr auto operator++() -> iterator&
        {
            flux::inc(*base_, cur_);
            return *this;
        }

        constexpr void operator++(int) { flux::inc(*base_, cur_); }

        constexpr auto operator++(int) -> iterator
            requires multipass_sequence<S>
        {
            auto temp = *this;
            ++*this;
            return temp;
        }

        constexpr auto operator--() -> iterator&
            requires bidirectional_sequence<S>
        {
            flux::dec(*base_, cur_);
            return *this;
        }

        constexpr auto operator--(int) -> iterator
            requires bidirectional_sequence<S>
        {
            auto temp = *this;
            --*this;
            return temp;
        }

        constexpr auto operator+=(difference_type n) -> iterator&
            requires random_access_sequence<S>
        {
            flux::inc(*base_, cur_, n);
            return *this;
        }

        constexpr auto operator-=(difference_type n) -> iterator&
            requires random_access_sequence<S>
        {
            flux::inc(*base_, cur_, -n);
            return *this;
        }

        constexpr auto operator[](difference_type n) const -> element_t<S>
            requires random_access_sequence<S>
        {
            auto i = flux::first(*base_);
            flux::inc(*base_, i, n);
            return flux::read_at(*base_, i);
        }

        constexpr auto operator->() const -> std::add_pointer_t<element_t<S>>
            requires contiguous_sequence<S>
        {
            return flux::data(*base_) + flux::distance(*base_, flux::first(*base_), cur_);
        }

        friend constexpr bool operator==(iterator const& self, std::default_sentinel_t)
        {
            return flux::is_last(*self.base_, self.cur_);
        }

        friend bool operator==(iterator const&, iterator const&)
            requires multipass_sequence<S>
            = default;

        friend std::strong_ordering operator<=>(iterator const&, iterator const&)
            requires random_access_sequence<S>
            = default;

        friend constexpr auto operator+(iterator self, difference_type n) -> iterator
            requires random_access_sequence<S>
        {
            flux::inc(*self.base_, self.cur_, n);
            return self;
        }

        friend constexpr auto operator+(difference_type n, iterator self) -> iterator
            requires random_access_sequence<S>
        {
            flux::inc(*self.base_, self.cur_, n);
            return self;
        }

        friend constexpr auto operator-(iterator self, difference_type n) -> iterator
            requires random_access_sequence<S>
        {
            flux::inc(*self.base_, self.cur_, -n);
            return self;
        }

        friend constexpr auto operator-(iterator const& lhs, iterator const& rhs)
            -> difference_type
            requires random_access_sequence<S>
        {
            return flux::distance(*lhs.base_, rhs.cur_, lhs.cur_);
        }

        friend constexpr auto iter_move(iterator const& self)
            -> rvalue_element_t<S>
        {
            return flux::move_at(*self.base_, self.cur_);
        }

        friend constexpr void iter_swap(iterator const& lhs, iterator const& rhs)
            requires element_swappable_with<S, S>
        {
            flux::swap_with(*lhs.base_, lhs.cur_, *rhs.base_, rhs.cur_);
        }
    };

public:
    constexpr view_adaptor() requires std::default_initializable<Base> = default;

    constexpr explicit view_adaptor(Base&& base)
        : base_(std::move(base))
    {}

    constexpr auto begin()
    {
        return iterator<false>(base_, flux::first(base_));
    }

    constexpr auto begin() const
        requires sequence<Base const>
    {
        return iterator<true>(base_, flux::first(base_));
    }

    constexpr auto end()
    {
        // Ranges requires sentinels to be copy-constructible
        if constexpr (bounded_sequence<Base> &&
                      std::copy_constructible<cursor_t<Base>>) {
            return iterator<false>(base_, flux::last(base_));
        } else {
            return std::default_sentinel;
        }
    }

    constexpr auto end() const
        requires sequence<Base const>
    {
        if constexpr (bounded_sequence<Base const> &&
                      std::copy_constructible<cursor_t<Base const>>) {
            return iterator<true>(base_, flux::last(base_));
        } else {
            return std::default_sentinel;
        }
    }

    constexpr auto size() requires sized_sequence<Base>
    {
        return flux::size(base_);
    }

    constexpr auto size() const requires sized_sequence<Base const>
    {
        return flux::size(base_);
    }

    constexpr auto data() requires contiguous_sequence<Base>
    {
        return flux::data(base_);
    }

    constexpr auto data() const requires contiguous_sequence<Base const>
    {
        return flux::data(base_);
    }
};

struct view_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr (std::ranges::viewable_range<Seq>) {
            return std::views::all(FLUX_FWD(seq));
        } else {
            return view_adaptor(flux::from(FLUX_FWD(seq)));
        }
    }
};

} // namespace detail

inline constexpr auto view = detail::view_fn{};

template <typename D>
constexpr auto lens_base<D>::view() &
{
    return flux::view(derived());
}

template <typename D>
constexpr auto lens_base<D>::view() const& requires sequence<D const>
{
    return flux::view(derived());
}

template <typename D>
constexpr auto lens_base<D>::view() &&
{
    return flux::view(std::move(derived()));
}

template <typename D>
constexpr auto lens_base<D>::view() const&& requires sequence<D const>
{
    return flux::view(std::move(derived()));
}

} // namespace flux

#endif // FLUX_RANGE_VIEW_HPP_INCLUDED

#endif // FLUX_RANGES_HPP_INCLUDED


#endif
