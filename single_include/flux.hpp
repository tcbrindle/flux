
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


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_UTILS_HPP_INCLUDED
#define FLUX_CORE_UTILS_HPP_INCLUDED

#include <compare>
#include <concepts>
#include <type_traits>


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_ASSERT_HPP_INCLUDED
#define FLUX_CORE_ASSERT_HPP_INCLUDED


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_CONFIG_HPP_INCLUDED
#define FLUX_CORE_CONFIG_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_MACROS_HPP_INCLUDED
#define FLUX_CORE_MACROS_HPP_INCLUDED

#include <version>

#define FLUX_VERSION_MAJOR 0
#define FLUX_VERSION_MINOR 4
#define FLUX_VERSION_PATCH 0
#define FLUX_VERSION_DEVEL 1 // 0 => Release, 1 => development post Major.Minor.Patch

#define FLUX_VERSION \
    (FLUX_VERSION_MAJOR * 100'000 \
    + FLUX_VERSION_MINOR * 1'000  \
    + FLUX_VERSION_PATCH * 10   \
    + FLUX_VERSION_DEVEL)

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

#define FLUX_ASSERT(cond) (::flux::assert_(cond, "assertion '" #cond "' failed"))

#define FLUX_DEBUG_ASSERT(cond) (::flux::assert_(!::flux::config::enable_debug_asserts || (cond), "assertion '" #cond "' failed"));

#ifdef FLUX_MODULE_INTERFACE
#define FLUX_EXPORT export
#else
#define FLUX_EXPORT
#endif

#endif // FLUX_CORE_MACROS_HPP_INCLUDED


#include <concepts>
#include <cstddef>
#include <type_traits>

#define FLUX_ERROR_POLICY_TERMINATE 1
#define FLUX_ERROR_POLICY_UNWIND     2

#define FLUX_OVERFLOW_POLICY_ERROR   10
#define FLUX_OVERFLOW_POLICY_WRAP    11
#define FLUX_OVERFLOW_POLICY_IGNORE  12

#define FLUX_DIVIDE_BY_ZERO_POLICY_ERROR   100
#define FLUX_DIVIDE_BY_ZERO_POLICY_IGNORE  101

// Default error policy is terminate
#define FLUX_DEFAULT_ERROR_POLICY FLUX_ERROR_POLICY_TERMINATE

// Default overflow policy is error in debug builds, wrap in release builds
#ifdef NDEBUG
#  define FLUX_DEFAULT_OVERFLOW_POLICY FLUX_OVERFLOW_POLICY_WRAP
#else
#  define FLUX_DEFAULT_OVERFLOW_POLICY FLUX_OVERFLOW_POLICY_ERROR
#endif // NDEBUG

// Default divide by zero policy is error in debug builds, ignore in release builds
#ifdef NDEBUG
#  define FLUX_DEFAULT_DIVIDE_BY_ZERO_POLICY FLUX_DIVIDE_BY_ZERO_POLICY_IGNORE
#else
#  define FLUX_DEFAULT_DIVIDE_BY_ZERO_POLICY FLUX_DIVIDE_BY_ZERO_POLICY_ERROR
#endif // NDEBUG

// Select which error policy to use
#if defined(FLUX_TERMINATE_ON_ERROR)
#  define FLUX_ERROR_POLICY FLUX_ERROR_POLICY_TERMINATE
#elif defined(FLUX_UNWIND_ON_ERROR)
#  define FLUX_ERROR_POLICY FLUX_ERROR_POLICY_UNWIND
#else
#  define FLUX_ERROR_POLICY FLUX_DEFAULT_ERROR_POLICY
#endif // FLUX_TERMINATE_ON_ERROR

// Should we print an error message before terminating?
#ifndef FLUX_PRINT_ERROR_ON_TERMINATE
#  define FLUX_PRINT_ERROR_ON_TERMINATE 1
#endif // FLUX_PRINT_ERROR_ON_TERMINATE

// Should we test debug assertions?
#ifndef FLUX_ENABLE_DEBUG_ASSERTS
#  ifdef NDEBUG
#    define FLUX_ENABLE_DEBUG_ASSERTS 0
#  else
#    define FLUX_ENABLE_DEBUG_ASSERTS 1
#  endif
#endif

// Select which overflow policy to use
#if defined(FLUX_ERROR_ON_OVERFLOW)
#  define FLUX_OVERFLOW_POLICY FLUX_OVERFLOW_POLICY_ERROR
#elif defined(FLUX_WRAP_ON_OVERFLOW)
#  define FLUX_OVERFLOW_POLICY FLUX_OVERFLOW_POLICY_WRAP
#elif defined(FLUX_IGNORE_OVERFLOW)
#  define FLUX_OVERFLOW_POLICY FLUX_OVERFLOW_POLICY_IGNORE
#else
#  define FLUX_OVERFLOW_POLICY FLUX_DEFAULT_OVERFLOW_POLICY
#endif // FLUX_ERROR_ON_OVERFLOW

// Select which overflow policy to use
#if defined(FLUX_ERROR_ON_DIVIDE_BY_ZERO)
#  define FLUX_DIVIDE_BY_ZERO_POLICY FLUX_DIVIDE_BY_ZERO_POLICY_ERROR
#elif defined(FLUX_IGNORE_DIVIDE_BY_ZERO)
#  define FLUX_DIVIDE_BY_ZERO_POLICY FLUX_DIVIDE_BY_ZERO_POLICY_IGNORE
#else
#  define FLUX_DIVIDE_BY_ZERO_POLICY FLUX_DEFAULT_DIVIDE_BY_ZERO_POLICY
#endif // FLUX_ERROR_ON_DIVIDE_BY_ZERO

// Should we try to use static bounds checking?
#if !defined(FLUX_DISABLE_STATIC_BOUNDS_CHECKING)
#  if defined(__has_cpp_attribute) && defined(__has_builtin)
#    if __has_builtin(__builtin_constant_p) && __has_cpp_attribute(gnu::error)
#      define FLUX_HAVE_GCC_STATIC_BOUNDS_CHECKING 1
#    endif
#  endif
#endif // FLUX_DISABLE_STATIC_BOUNDS_CHECKING

// Default int_t is ptrdiff_t
#define FLUX_DEFAULT_INT_TYPE std::ptrdiff_t

// Select which int type to use
#ifndef FLUX_INT_TYPE
#define FLUX_INT_TYPE FLUX_DEFAULT_INT_TYPE
#endif

namespace flux {

FLUX_EXPORT
enum class error_policy {
    terminate = FLUX_ERROR_POLICY_TERMINATE,
    unwind = FLUX_ERROR_POLICY_UNWIND
};

FLUX_EXPORT
enum class overflow_policy {
    ignore = FLUX_OVERFLOW_POLICY_IGNORE,
    wrap = FLUX_OVERFLOW_POLICY_WRAP,
    error = FLUX_OVERFLOW_POLICY_ERROR
};

enum class divide_by_zero_policy {
    ignore = FLUX_DIVIDE_BY_ZERO_POLICY_IGNORE,
    error = FLUX_DIVIDE_BY_ZERO_POLICY_ERROR
};

namespace config {

FLUX_EXPORT
using int_type = FLUX_INT_TYPE;
static_assert(std::signed_integral<int_type> && (sizeof(int_type) >= sizeof(std::ptrdiff_t)),
              "Custom FLUX_INT_TYPE must be a signed integer type at least as large as ptrdiff_t");

FLUX_EXPORT
inline constexpr error_policy on_error = static_cast<error_policy>(FLUX_ERROR_POLICY);

FLUX_EXPORT
inline constexpr overflow_policy on_overflow = static_cast<overflow_policy>(FLUX_OVERFLOW_POLICY);

FLUX_EXPORT
inline constexpr divide_by_zero_policy on_divide_by_zero = static_cast<divide_by_zero_policy>(FLUX_DIVIDE_BY_ZERO_POLICY);

FLUX_EXPORT
inline constexpr bool print_error_on_terminate = FLUX_PRINT_ERROR_ON_TERMINATE;

FLUX_EXPORT
inline constexpr bool enable_debug_asserts = FLUX_ENABLE_DEBUG_ASSERTS;

} // namespace config

} // namespace flux

#endif


#include <cstdio>
#include <exception>
#include <source_location>
#include <stdexcept>
#include <type_traits>

namespace flux {

FLUX_EXPORT
struct unrecoverable_error : std::logic_error {
    explicit inline unrecoverable_error(char const* msg) : std::logic_error(msg) {}
};

namespace detail {

struct runtime_error_fn {
    [[noreturn]]
    void operator()(char const* msg,
                    std::source_location loc = std::source_location::current()) const
    {
        if constexpr (config::on_error == error_policy::unwind) {
            char buf[1024];
            std::snprintf(buf, 1024, "%s:%u: Fatal error: %s",
                          loc.file_name(), loc.line(), msg);
            throw unrecoverable_error(buf);
        } else {
            if constexpr (config::print_error_on_terminate) {
                std::fprintf(stderr, "%s:%u: Fatal error: %s\n",
                             loc.file_name(), loc.line(), msg);
            }
            std::terminate();
        }
    }
};

}

FLUX_EXPORT inline constexpr auto runtime_error = detail::runtime_error_fn{};

#ifdef FLUX_HAVE_GCC_STATIC_BOUNDS_CHECKING
[[gnu::error("out-of-bounds sequence access detected")]]
void static_bounds_check_failed(); // not defined
#endif

namespace detail {

struct assert_fn {
    inline constexpr void operator()(bool cond, char const* msg,
                              std::source_location loc = std::source_location::current()) const
    {
        if (cond) {
            return;
        } else {
            runtime_error(msg, std::move(loc));
        }
    }
};

struct bounds_check_fn {
    inline constexpr void operator()(bool cond, std::source_location loc = std::source_location::current()) const
    {
        if (!std::is_constant_evaluated()) {
            assert_fn{}(cond, "out of bounds sequence access", std::move(loc));
        }
    }
};

struct indexed_bounds_check_fn {
    template <typename T>
    inline constexpr void operator()(T idx, T limit,
                                     std::source_location loc = std::source_location::current()) const
    {
        if (!std::is_constant_evaluated()) {
#ifdef FLUX_HAVE_GCC_STATIC_BOUNDS_CHECKING
            if (__builtin_constant_p(idx) && __builtin_constant_p(limit)) {
                if (idx < T{0} || idx >= limit) {
                    static_bounds_check_failed();
                }
            }
#endif
            assert_fn{}(idx >= T{0}, "index cannot be negative", loc);
            assert_fn{}(idx < limit, "out-of-bounds sequence access", loc);
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto assert_ = detail::assert_fn{};
FLUX_EXPORT inline constexpr auto bounds_check = detail::bounds_check_fn{};
FLUX_EXPORT inline constexpr auto indexed_bounds_check = detail::indexed_bounds_check_fn{};

} // namespace flux

#endif // FLUX_CORE_ASSERT_HPP_INCLUDED


namespace flux {

/*
 * Useful helpers
 */
FLUX_EXPORT
template <typename From, typename To>
concept decays_to = std::same_as<std::remove_cvref_t<From>, To>;

FLUX_EXPORT
template <typename T, typename U>
concept same_decayed = std::same_as<std::remove_cvref_t<T>,
                                    std::remove_cvref_t<U>>;

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

} // namespace detail

FLUX_EXPORT inline constexpr auto copy = detail::copy_fn{};

namespace detail {

template <std::integral To>
struct checked_cast_fn {
    template <std::integral From>
    [[nodiscard]]
    constexpr auto operator()(From from) const -> To
    {
        if constexpr (requires { To{from}; }) {
            return To{from}; // not a narrowing conversion
        } else {
            To to = static_cast<To>(from);
            FLUX_DEBUG_ASSERT(static_cast<From>(to) == from);
            if constexpr (std::is_signed_v<From> != std::is_signed_v<To>) {
                FLUX_DEBUG_ASSERT((to < To{}) == (from < From{}));
            }
            return to;
        }
    }
};

} // namespace detail

FLUX_EXPORT
template <std::integral To>
inline constexpr auto checked_cast = detail::checked_cast_fn<To>{};

namespace detail {

template <typename T, typename... U>
concept any_of = (std::same_as<T, U> || ...);

template <typename T, typename Cat>
concept compares_as = std::same_as<std::common_comparison_category_t<T, Cat>, Cat>;

template <typename Fn, typename T, typename U, typename Cat>
concept ordering_invocable_ =
    std::regular_invocable<Fn, T, U> &&
    compares_as<std::decay_t<std::invoke_result_t<Fn, T, U>>, Cat>;

} // namespace detail

FLUX_EXPORT
template <typename Fn, typename T, typename U, typename Cat = std::partial_ordering>
concept ordering_invocable =
    detail::ordering_invocable_<Fn, T, U, Cat> &&
    detail::ordering_invocable_<Fn, U, T, Cat> &&
    detail::ordering_invocable_<Fn, T, T, Cat> &&
    detail::ordering_invocable_<Fn, U, U, Cat>;

} // namespace flux

#endif


#include <compare>
#include <concepts>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <type_traits>

// clang-format off

// Workaround GCC11/12 ICE in sequence concept definition below
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


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED
#define FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_NUMERIC_HPP_INCLUDED
#define FLUX_CORE_NUMERIC_HPP_INCLUDED



#include <limits>

namespace flux::num {

FLUX_EXPORT
inline constexpr auto wrapping_add = []<std::signed_integral T>(T lhs, T rhs) -> T
{
    using U = std::make_unsigned_t<T>;
    return static_cast<T>(static_cast<U>(lhs) + static_cast<U>(rhs));
};

FLUX_EXPORT
inline constexpr auto wrapping_sub = []<std::signed_integral T>(T lhs, T rhs) -> T
{
    using U = std::make_unsigned_t<T>;
    return static_cast<T>(static_cast<U>(lhs) - static_cast<U>(rhs));
};

FLUX_EXPORT
inline constexpr auto wrapping_mul = []<std::signed_integral T>(T lhs, T rhs) -> T
{
    using U = std::make_unsigned_t<T>;
    return static_cast<T>(static_cast<U>(lhs) * static_cast<U>(rhs));
};

namespace detail {

#if defined(__has_builtin)
#  if __has_builtin(__builtin_add_overflow) && \
      __has_builtin(__builtin_sub_overflow) && \
      __has_builtin(__builtin_mul_overflow)
#    define FLUX_HAVE_BUILTIN_OVERFLOW_OPS 1
#  endif
#endif

#ifdef FLUX_HAVE_BUILTIN_OVERFLOW_OPS
    inline constexpr bool use_builtin_overflow_ops = true;
#else
    inline constexpr bool use_builtin_overflow_ops = false;
#endif

#undef FLUX_HAVE_BUILTIN_OVERFLOW_OPS

}

FLUX_EXPORT
template <std::signed_integral T>
struct overflow_result {
    T value;
    bool overflowed;
};

FLUX_EXPORT
inline constexpr auto overflowing_add = []<std::signed_integral T>(T lhs, T rhs)
    -> overflow_result<T>
{
    if constexpr (detail::use_builtin_overflow_ops) {
        bool overflowed = __builtin_add_overflow(lhs, rhs, &lhs);
        return {lhs, overflowed};
    } else {
        T value = wrapping_add(lhs, rhs);
        bool overflowed = ((lhs < T{}) == (rhs < T{})) && ((lhs < T{}) != (value < T{}));
        return {value, overflowed};
    }
};

FLUX_EXPORT
inline constexpr auto overflowing_sub = []<std::signed_integral T>(T lhs, T rhs)
    -> overflow_result<T>
{
    if constexpr (detail::use_builtin_overflow_ops) {
        bool overflowed = __builtin_sub_overflow(lhs, rhs, &lhs);
        return {lhs, overflowed};
    } else {
        T value = wrapping_sub(lhs, rhs);
        bool overflowed = (lhs >= T{} && rhs < T{} && value < T{}) ||
                         (lhs < T{} && rhs > T{} && value > T{});
        return {value, overflowed};
    }
};

FLUX_EXPORT
inline constexpr auto overflowing_mul = []<std::signed_integral T>(T lhs, T rhs)
    -> overflow_result<T>
{
    if constexpr (detail::use_builtin_overflow_ops) {
        bool overflowed = __builtin_mul_overflow(lhs, rhs, &lhs);
        return {lhs, overflowed};
    } else {
        T value =  wrapping_mul(lhs, rhs);
        return {value, lhs != 0 && value/lhs != rhs};
    }
};

FLUX_EXPORT
inline constexpr auto checked_add =
    []<std::signed_integral T>(T lhs, T rhs,
                               std::source_location loc = std::source_location::current())
    -> T
{
    if (std::is_constant_evaluated()) {
        return lhs + rhs;
    } else {
        if constexpr (config::on_overflow == overflow_policy::ignore) {
            return lhs + rhs;
        } else if constexpr (config::on_overflow == overflow_policy::wrap) {
            return wrapping_add(lhs, rhs);
        } else {
            auto res = overflowing_add(lhs, rhs);
            if (res.overflowed) {
                runtime_error("signed overflow in addition", loc);
            }
            return res.value;
        }
    }
};

FLUX_EXPORT
inline constexpr auto checked_sub =
    []<std::signed_integral T>(T lhs, T rhs,
                               std::source_location loc = std::source_location::current())
    -> T
{
  if (std::is_constant_evaluated()) {
      return lhs - rhs;
  } else {
      if constexpr (config::on_overflow == overflow_policy::ignore) {
          return lhs - rhs;
      } else if constexpr (config::on_overflow == overflow_policy::wrap) {
          return wrapping_sub(lhs, rhs);
      } else {
          auto res = overflowing_sub(lhs, rhs);
          if (res.overflowed) {
              runtime_error("signed overflow in subtraction", loc);
          }
          return res.value;
      }
  }
};

FLUX_EXPORT
inline constexpr auto checked_mul =
    []<std::signed_integral T>(T lhs, T rhs,
                               std::source_location loc = std::source_location::current())
    -> T
{
  if (std::is_constant_evaluated()) {
      return lhs * rhs;
  } else {
      if constexpr (config::on_overflow == overflow_policy::ignore) {
          return lhs * rhs;
      } else if constexpr (config::on_overflow == overflow_policy::wrap) {
          return wrapping_mul(lhs, rhs);
      } else {
          auto res = overflowing_mul(lhs, rhs);
          if (res.overflowed) {
              runtime_error("signed overflow in multiplication", loc);
          }
          return res.value;
      }
  }
};

FLUX_EXPORT
inline constexpr auto checked_pow =
        []<std::signed_integral T, std::unsigned_integral U>(T base, U exponent,
                                   std::source_location loc = std::source_location::current())
                -> T
{
    T res{1};
    for(U i{0}; i < exponent; i++) {
        res = checked_mul(res, base, loc);
    }
    return res;
};


inline constexpr auto checked_div =
    []<std::signed_integral T>(T lhs, T rhs,
                               std::source_location loc = std::source_location::current())
    -> T
{
    if (std::is_constant_evaluated()) {
        return lhs / rhs;
    } else {
        if constexpr (config::on_divide_by_zero == divide_by_zero_policy::ignore) {
            return lhs / rhs;
        } else {
            if (rhs == 0) {
                runtime_error("divide by zero", loc);
            }
            return lhs / rhs;
        }
    }
};

inline constexpr auto checked_mod =
    []<std::signed_integral T>(T lhs, T rhs,
                               std::source_location loc = std::source_location::current())
    -> T
{
    if (std::is_constant_evaluated()) {
        return lhs % rhs;
    } else {
        if constexpr (config::on_divide_by_zero == divide_by_zero_policy::ignore) {
            return lhs % rhs;
        } else {
            if (rhs == 0) {
                runtime_error("divide by zero", loc);
            }
            return lhs % rhs;
        }
    }
};

} // namespace flux::num

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_SEQUENCE_ACCESS_HPP_INCLUDED
#define FLUX_CORE_SEQUENCE_ACCESS_HPP_INCLUDED



// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_OPTIONAL_HPP_INCLUDED
#define FLUX_CORE_OPTIONAL_HPP_INCLUDED




#include <functional>
#include <optional>

namespace flux {

FLUX_EXPORT using nullopt_t = std::nullopt_t;
FLUX_EXPORT inline constexpr nullopt_t const& nullopt = std::nullopt;

namespace detail {

template <typename T>
concept can_optional =
    (std::is_object_v<T> || std::is_lvalue_reference_v<T>) &&
    !decays_to<T, nullopt_t> &&
    !decays_to<T, std::in_place_t>;

}

FLUX_EXPORT
template <typename T>
class optional;

template <detail::can_optional T>
    requires std::is_object_v<T>
class optional<T> {

    struct dummy {};

    union {
        dummy dummy_{};
        T item_;
    };

    bool engaged_ = false;

    template <typename... Args>
    constexpr auto construct(Args&&... args) {
        std::construct_at(std::addressof(item_), FLUX_FWD(args)...);
        engaged_ = true;
    }

public:

    constexpr optional() noexcept {}

    constexpr explicit(false) optional(nullopt_t) noexcept {}

    template <decays_to<T> U = T>
    constexpr explicit optional(U&& item)
        noexcept(std::is_nothrow_constructible_v<T, U>)
        : item_(FLUX_FWD(item)),
          engaged_(true)
    {}

    template <typename... Args>
        requires std::constructible_from<T, Args...>
    constexpr explicit optional(std::in_place_t, Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : item_(FLUX_FWD(args)...),
          engaged_(true)
    {}

    /*
     * Destructors
     */
    constexpr ~optional()
    {
        if (engaged_) {
            item_.T::~T();
        }
    }

    ~optional() requires std::is_trivially_destructible_v<T> = default;

    /*
     * Copy constructors
     */
    constexpr optional(optional const& other)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires std::copy_constructible<T>
    {
        if (other.engaged_) {
            construct(other.item_);
        }
    }

    optional(optional const&)
        requires std::copy_constructible<T> &&
                 std::is_trivially_constructible_v<T>
        = default;

    /*
     * Copy-assignment operators
     */
    constexpr optional& operator=(optional const& other)
        noexcept(std::is_nothrow_copy_assignable_v<T> &&
                 std::is_nothrow_copy_constructible_v<T>)
        requires std::copy_constructible<T>
    {
        if (engaged_) {
            if (other.engaged_) {
                if constexpr (std::is_copy_assignable_v<T>) {
                    item_ = other.item_;
                } else {
                    reset();
                    construct(other.item_);
                }
            } else {
                reset();
            }
        } else {
            if (other.engaged_) {
                construct(other.item_);
            }
        }
        return *this;
    }

    optional& operator=(optional const&)
        requires std::copy_constructible<T> &&
                 std::is_trivially_copy_assignable_v<T>
        = default;

    /*
     * Move constructors
     */
    constexpr optional(optional&& other)
        noexcept(std::is_nothrow_move_constructible_v<T>)
        requires std::move_constructible<T>
    {
        if (other.engaged_) {
            construct(std::move(other).item_);
        }
    }

    optional(optional&&)
        requires std::move_constructible<T> &&
                 std::is_trivially_move_constructible_v<T>
        = default;

    /*
     * Move-assignment operators
     */
    constexpr optional& operator=(optional&& other)
        noexcept(std::is_nothrow_move_constructible_v<T> &&
                 std::is_nothrow_move_assignable_v<T>)
        requires std::move_constructible<T>
    {
        if (engaged_) {
            if (other.engaged_) {
                if constexpr (std::is_move_assignable_v<T>) {
                    item_ = std::move(other).item_;
                } else {
                    reset();
                    construct(std::move(other).item_);
                }
            } else {
                reset();
            }
        } else {
            if (other.engaged_) {
                construct(std::move(other).item_);
            }
        }
        return *this;
    }

    constexpr optional& operator=(optional&&)
        requires std::move_constructible<T> &&
                 std::is_trivially_move_assignable_v<T>
        = default;

    /*
     * Observers
     */
    [[nodiscard]]
    constexpr auto has_value() const -> bool { return engaged_; }

    constexpr explicit operator bool() const { return engaged_; }

    template <decays_to<optional> Opt>
    [[nodiscard]]
    friend constexpr auto operator*(Opt&& self) -> decltype(auto)
    {
        if (!std::is_constant_evaluated()) {
            FLUX_ASSERT(self.has_value());
        }
        return (FLUX_FWD(self).item_);
    }

    constexpr auto operator->() -> T*
    {
        if (!std::is_constant_evaluated()) {
            FLUX_ASSERT(this->has_value());
        }
        return std::addressof(item_);
    }

    constexpr auto operator->() const -> T const*
    {
        if (!std::is_constant_evaluated()) {
            FLUX_ASSERT(this->has_value());
        }
        return std::addressof(item_);
    }

    [[nodiscard]] constexpr auto value() & -> T& { return **this; }
    [[nodiscard]] constexpr auto value() const& -> T const& { return **this; }
    [[nodiscard]] constexpr auto value() && -> T&& { return *std::move(*this); }
    [[nodiscard]] constexpr auto value() const&& -> T const&& { return *std::move(*this); }

    [[nodiscard]] constexpr auto value_unchecked() & noexcept -> T& { return item_; }
    [[nodiscard]] constexpr auto value_unchecked() const& noexcept -> T const& { return item_; }
    [[nodiscard]] constexpr auto value_unchecked() && noexcept -> T&& { return std::move(item_); }
    [[nodiscard]] constexpr auto value_unchecked() const&& noexcept -> T const&& { return std::move(item_); }

    [[nodiscard]] constexpr auto value_or(auto&& alt) &
        -> decltype(has_value() ? value_unchecked() : FLUX_FWD(alt))
    {
        return has_value() ? value_unchecked() : FLUX_FWD(alt);
    }

    [[nodiscard]] constexpr auto value_or(auto&& alt) const&
        -> decltype(has_value() ? value_unchecked() : FLUX_FWD(alt))
    {
        return has_value() ? value_unchecked() : FLUX_FWD(alt);
    }

    [[nodiscard]] constexpr auto value_or(auto&& alt) &&
        -> decltype(has_value() ? std::move(*this).value_unchecked() : FLUX_FWD(alt))
    {
        return has_value() ? std::move(*this).value_unchecked() : FLUX_FWD(alt);
    }

    [[nodiscard]] constexpr auto value_or(auto&& alt) const&&
        -> decltype(has_value() ? std::move(*this).value_unchecked() : FLUX_FWD(alt))
    {
        return has_value() ? std::move(*this).value_unchecked() : FLUX_FWD(alt);
    }

private:
    template <typename Cmp>
    static constexpr auto do_compare(optional const& lhs, optional const& rhs, Cmp cmp)
        -> std::decay_t<std::invoke_result_t<Cmp&, T const&, T const&>>
    {
        if (lhs.has_value() && rhs.has_value()) {
            return cmp(lhs.value_unchecked(), rhs.value_unchecked());
        } else {
            return cmp(lhs.has_value(), rhs.has_value());
        }
    }

public:
    [[nodiscard]]
    friend constexpr auto operator==(optional const& lhs, optional const& rhs) -> bool
        requires std::equality_comparable<T>
    {
        return do_compare(lhs, rhs, std::equal_to{});
    }

    [[nodiscard]]
    friend constexpr auto operator<=>(optional const& lhs, optional const& rhs)
        requires std::totally_ordered<T> && std::three_way_comparable<T>
    {
        return do_compare(lhs, rhs, std::compare_three_way{});
    }

    [[nodiscard]]
    friend constexpr auto operator<=>(optional const& lhs, optional const& rhs)
        requires std::totally_ordered<T>
    {
        return do_compare(lhs, rhs, std::compare_partial_order_fallback);
    }

    [[nodiscard]]
    friend constexpr auto operator==(optional const& o, nullopt_t) -> bool
    {
        return !o.has_value();
    }

    [[nodiscard]]
    friend constexpr auto operator<=>(optional const& o, nullopt_t)
        -> std::strong_ordering
    {
        return o.has_value() ? std::strong_ordering::greater
                             : std::strong_ordering::equivalent;
    }

    /*
     * Modifiers
     */

    constexpr auto reset() -> void
    {
        if (engaged_) {
            item_.T::~T();
            engaged_ = false;
        }
    }

    template <typename... Args>
        requires std::constructible_from<T, Args...>
    constexpr auto emplace(Args&&... args) -> T&
    {
        reset();
        construct(FLUX_FWD(args)...);
        return item_;
    }

    /*
     * Monadic operations
     */
    template <typename F>
        requires std::invocable<F, T&> &&
                 detail::can_optional<std::invoke_result_t<F, T&>>
    [[nodiscard]]
    constexpr auto map(F&& func) & -> optional<std::invoke_result_t<F, T&>>
    {
        using R = optional<std::invoke_result_t<F, T&>>;
        if (engaged_) {
            return R(std::invoke(FLUX_FWD(func), value_unchecked()));
        } else {
            return nullopt;
        }
    }

    template <typename F>
        requires std::invocable<F, T const&> &&
                 detail::can_optional<std::invoke_result_t<F, T const&>>
    [[nodiscard]]
    constexpr auto map(F&& func) const& -> optional<std::invoke_result_t<F, T const&>>
    {
        using R = optional<std::invoke_result_t<F, T const&>>;
        if (engaged_) {
            return R(std::invoke(FLUX_FWD(func), value_unchecked()));
        } else {
            return nullopt;
        }
    }

    template <typename F>
        requires std::invocable<F, T&&> &&
                 detail::can_optional<std::invoke_result_t<F, T&&>>
    [[nodiscard]]
    constexpr auto map(F&& func) && -> optional<std::invoke_result_t<F, T&&>>
    {
        using R = optional<std::invoke_result_t<F, T&>>;
        if (engaged_) {
            return R(std::invoke(FLUX_FWD(func), std::move(*this).value_unchecked()));
        } else {
            return nullopt;
        }
    }

    template <typename F>
        requires std::invocable<F, T const&&> &&
                 detail::can_optional<std::invoke_result_t<F, T const&&>>
    [[nodiscard]]
    constexpr auto map(F&& func) const&& -> optional<std::invoke_result_t<F, T const&&>>
    {
        using R = optional<std::invoke_result_t<F, T const&&>>;
        if (engaged_) {
            return R(std::invoke(FLUX_FWD(func), std::move(*this).value_unchecked()));
        } else {
            return nullopt;
        }
    }
};

template <typename T>
optional(T) -> optional<T>;

template <detail::can_optional T>
class optional<T&> {
    T* ptr_ = nullptr;

    static void test_fn(T&);

public:
    optional() = default;

    constexpr explicit(false) optional(nullopt_t) noexcept {};

    template <typename U = T>
        requires requires(U& u) { test_fn(u); }
    constexpr explicit optional(U& item) noexcept
        : ptr_(std::addressof(item))
    {}

    /*
     * Observers
     */
    [[nodiscard]]
    constexpr auto has_value() const noexcept { return ptr_ != nullptr; }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept { return ptr_ != nullptr; }

    [[nodiscard]]
    constexpr auto operator*() const -> T&
    {
        if (!std::is_constant_evaluated()) {
            FLUX_ASSERT(ptr_ != nullptr);
        }
        return *ptr_;
    }

    constexpr auto operator->() const -> T*
    {
        if (!std::is_constant_evaluated()) {
            FLUX_ASSERT(ptr_ != nullptr);
        }
        return ptr_;
    }

    [[nodiscard]]
    constexpr auto value() const -> T& { return **this; }

    [[nodiscard]]
    constexpr auto value_unchecked() const noexcept -> T& { return *ptr_; }

    [[nodiscard]]
    constexpr auto value_or(auto&& alt) const
        -> decltype(has_value() ? value_unchecked() : FLUX_FWD(alt))
    {
        return has_value() ? value_unchecked() : FLUX_FWD(alt);
    }

    [[nodiscard]]
    friend constexpr auto operator==(optional const& o, nullopt_t) -> bool
    {
        return !o.has_value();
    }

    [[nodiscard]]
    friend constexpr auto operator<=>(optional const& o, nullopt_t)
        -> std::strong_ordering
    {
        return o.has_value() ? std::strong_ordering::greater
                             : std::strong_ordering::equivalent;
    }

    /*
     * Modifiers
     */
    constexpr auto reset() -> void { ptr_ = nullptr; }

    template <typename U = T>
        requires requires(U& u) { test_fn(u); }
    constexpr auto emplace(U& item) -> void { ptr_ = std::addressof(item); }

    /*
     * Monadic operations
     */
    template <typename F>
        requires std::invocable<F, T&> &&
                 detail::can_optional<std::invoke_result_t<F, T&>>
    [[nodiscard]]
    constexpr auto map(F&& func) const -> optional<std::invoke_result_t<F, T&>>
    {
        using R = optional<std::invoke_result_t<F, T&>>;
        if (ptr_) {
            return R(std::invoke(FLUX_FWD(func), *ptr_));
        } else {
            return nullopt;
        }
    }
};

} // namespace flux

#endif


namespace flux {

namespace detail {

struct first_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const
        noexcept(noexcept(traits_t<Seq>::first(seq))) -> cursor_t<Seq>
    {
        return traits_t<Seq>::first(seq);
    }
};

struct is_last_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        noexcept(noexcept(traits_t<Seq>::is_last(seq, cur))) -> bool
    {
        return traits_t<Seq>::is_last(seq, cur);
    }
};

struct read_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        noexcept(noexcept(traits_t<Seq>::read_at(seq, cur))) -> element_t<Seq>
    {
        return traits_t<Seq>::read_at(seq, cur);
    }
};

struct inc_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur) const
        noexcept(noexcept(traits_t<Seq>::inc(seq, cur))) -> cursor_t<Seq>&
    {
        (void) traits_t<Seq>::inc(seq, cur);
        return cur;
    }

    template <random_access_sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur, distance_t offset) const
        noexcept(noexcept(traits_t<Seq>::inc(seq, cur, offset))) -> cursor_t<Seq>&
    {
        (void) traits_t<Seq>::inc(seq, cur, offset);
        return cur;
    }
};

struct dec_fn {
    template <bidirectional_sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur) const
        noexcept(noexcept(traits_t<Seq>::dec(seq, cur))) -> cursor_t<Seq>&
    {
        (void) traits_t<Seq>::dec(seq, cur);
        return cur;
    }
};

struct distance_fn {
    template <multipass_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& from,
                                            cursor_t<Seq> const& to) const
        -> distance_t
    {
        if constexpr (random_access_sequence<Seq>) {
            return traits_t<Seq>::distance(seq, from, to);
        } else {
            distance_t n = 0;
            auto from_ = from;
            while (from_ != to) {
                inc_fn{}(seq, from_);
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
        noexcept(noexcept(traits_t<Seq>::data(seq)))
    {
        return traits_t<Seq>::data(seq);
    }
};

struct last_fn {
    template <bounded_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const
        noexcept(noexcept(traits_t<Seq>::last(seq))) -> cursor_t<Seq>
    {
        return traits_t<Seq>::last(seq);
    }
};

struct size_fn {
    template <sized_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> distance_t
    {
        return traits_t<Seq>::size(seq);
    }
};

struct usize_fn {
    template <sized_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> std::size_t
    {
        return checked_cast<std::size_t>(size_fn{}(seq));
    }
};

struct move_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        -> rvalue_element_t<Seq>
    {
        return traits_t<Seq>::move_at(seq, cur);
    }
};

struct read_at_unchecked_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        -> element_t<Seq>
    {
        return traits_t<Seq>::read_at_unchecked(seq, cur);
    }
};

struct move_at_unchecked_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        -> rvalue_element_t<Seq>
    {
        return traits_t<Seq>::move_at_unchecked(seq, cur);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto first = detail::first_fn{};
FLUX_EXPORT inline constexpr auto is_last = detail::is_last_fn{};
FLUX_EXPORT inline constexpr auto read_at = detail::read_at_fn{};
FLUX_EXPORT inline constexpr auto move_at = detail::move_at_fn{};
FLUX_EXPORT inline constexpr auto read_at_unchecked = detail::read_at_unchecked_fn{};
FLUX_EXPORT inline constexpr auto move_at_unchecked = detail::move_at_unchecked_fn{};
FLUX_EXPORT inline constexpr auto inc = detail::inc_fn{};
FLUX_EXPORT inline constexpr auto dec = detail::dec_fn{};
FLUX_EXPORT inline constexpr auto distance = detail::distance_fn{};
FLUX_EXPORT inline constexpr auto data = detail::data_fn{};
FLUX_EXPORT inline constexpr auto last = detail::last_fn{};
FLUX_EXPORT inline constexpr auto size = detail::size_fn{};
FLUX_EXPORT inline constexpr auto usize = detail::usize_fn{};

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
    constexpr auto operator()(Seq& seq, cursor_t<Seq> cur, distance_t offset) const
        -> cursor_t<Seq>
    {
        if constexpr (random_access_sequence<Seq>) {
            return inc(seq, cur, offset);
        } else if constexpr (bidirectional_sequence<Seq>) {
            auto const zero = distance_t{0};
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
            while (offset-- > distance_t{0}) {
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
    template <sequence Seq>
        requires (multipass_sequence<Seq> || sized_sequence<Seq>)
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> bool
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

struct front_fn {
    template <multipass_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const -> optional<element_t<Seq>>
    {
        auto cur = first(seq);
        if (!is_last(seq, cur)) {
            return optional<element_t<Seq>>(read_at(seq, cur));
        } else {
            return nullopt;
        }
    }
};

struct back_fn {
    template <bidirectional_sequence Seq>
        requires bounded_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const -> optional<element_t<Seq>>
    {
        auto cur = last(seq);
        if (cur != first(seq)) {
            return optional<element_t<Seq>>(read_at(seq, dec(seq, cur)));
        } else {
            return nullopt;
        }
    }
};

} // namespace detail


FLUX_EXPORT inline constexpr auto next = detail::next_fn{};
FLUX_EXPORT inline constexpr auto prev = detail::prev_fn{};
FLUX_EXPORT inline constexpr auto is_empty = detail::is_empty_fn{};
FLUX_EXPORT inline constexpr auto swap_with = detail::swap_with_fn{};
FLUX_EXPORT inline constexpr auto swap_at = detail::swap_at_fn{};
FLUX_EXPORT inline constexpr auto front = detail::front_fn{};
FLUX_EXPORT inline constexpr auto back = detail::back_fn{};

} // namespace flux

#endif


#include <functional>
#include <ranges>

namespace flux {

/*
 * Default implementation for C arrays of known bound
 */
template <typename T, index_t N>
struct sequence_traits<T[N]> : default_sequence_traits {

    static constexpr auto first(auto const&) -> index_t { return index_t{0}; }

    static constexpr bool is_last(auto const&, index_t idx) { return idx >= N; }

    static constexpr auto read_at(auto& self, index_t idx) -> decltype(auto)
    {
        indexed_bounds_check(idx, N);
        return self[idx];
    }

    static constexpr auto read_at_unchecked(auto& self, index_t idx) -> decltype(auto)
    {
        return self[idx];
    }

    static constexpr auto inc(auto const&, index_t& idx)
    {
        FLUX_DEBUG_ASSERT(idx < N);
        idx = num::checked_add(idx, distance_t{1});
    }

    static constexpr auto last(auto const&) -> index_t { return N; }

    static constexpr auto dec(auto const&, index_t& idx)
    {
        FLUX_DEBUG_ASSERT(idx > 0);
        idx = num::checked_sub(idx, distance_t{1});
    }

    static constexpr auto inc(auto const&, index_t& idx, distance_t offset)
    {
        FLUX_DEBUG_ASSERT(num::checked_add(idx, offset) <= N);
        FLUX_DEBUG_ASSERT(num::checked_add(idx, offset) >= 0);
        idx = num::checked_add(idx, offset);
    }

    static constexpr auto distance(auto const&, index_t from, index_t to) -> distance_t
    {
        return num::checked_sub(to, from);
    }

    static constexpr auto data(auto& self) -> auto* { return self; }

    static constexpr auto size(auto const&) -> distance_t { return N; }

    static constexpr auto for_each_while(auto& self, auto&& pred) -> index_t
    {
        index_t idx = 0;
        while (idx < N) {
            if (!std::invoke(pred, self[idx])) {
                break;
            }
            ++idx;
        }
        return idx;
    }
};

/*
 * Default implementation for std::reference_wrapper<T>
 */
template <sequence Seq>
struct sequence_traits<std::reference_wrapper<Seq>> : default_sequence_traits {

    using self_t = std::reference_wrapper<Seq>;

    using value_type = value_t<Seq>;

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

    static constexpr auto read_at_unchecked(self_t self, cursor_t<Seq> const& cur)
        -> decltype(auto)
    {
        return flux::read_at_unchecked(self.get(), cur);
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

    static constexpr auto inc(self_t self, cursor_t<Seq>& cur, distance_t offset)
        -> cursor_t<Seq>&
        requires random_access_sequence<Seq>
    {
        return flux::inc(self.get(), cur,  offset);
    }

    static constexpr auto distance(self_t self, cursor_t<Seq> const& from,
                                   cursor_t<Seq> const& to)
        -> distance_t
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

    static constexpr auto size(self_t self) -> distance_t
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

// Default implementation for contiguous, sized ranges
template <typename R>
    requires (!detail::derived_from_inline_sequence_base<R> &&
             std::ranges::contiguous_range<R> &&
             std::ranges::sized_range<R> &&
             std::ranges::contiguous_range<R const> &&
             std::ranges::sized_range<R const>)
struct sequence_traits<R> : default_sequence_traits {

    using value_type = std::ranges::range_value_t<R>;

    static constexpr auto first(auto&) -> index_t { return index_t{0}; }

    static constexpr auto is_last(auto& self, index_t idx)
    {
        return idx >= size(self);
    }

    static constexpr auto inc(auto& self, index_t& idx)
    {
        FLUX_DEBUG_ASSERT(idx < size(self));
        idx = num::checked_add(idx, distance_t{1});
    }

    static constexpr auto read_at(auto& self, index_t idx) -> decltype(auto)
    {
        indexed_bounds_check(idx, size(self));
        return data(self)[idx];
    }

    static constexpr auto read_at_unchecked(auto& self, index_t idx) -> decltype(auto)
    {
        return data(self)[idx];
    }

    static constexpr auto dec(auto&, index_t& idx)
    {
        FLUX_DEBUG_ASSERT(idx > 0);
        idx = num::checked_sub(idx, distance_t{1});
    }

    static constexpr auto last(auto& self) -> index_t { return size(self); }

    static constexpr auto inc(auto& self, index_t& idx, distance_t offset)
    {
        FLUX_DEBUG_ASSERT(num::checked_add(idx, offset) <= size(self));
        FLUX_DEBUG_ASSERT(num::checked_add(idx, offset) >= 0);
        idx = num::checked_add(idx, offset);
    }

    static constexpr auto distance(auto&, index_t from, index_t to) -> distance_t
    {
        return num::checked_sub(to, from);
    }

    static constexpr auto size(auto& self) -> distance_t
    {
        return checked_cast<distance_t>(std::ranges::ssize(self));
    }

    static constexpr auto data(auto& self) -> auto*
    {
        return std::ranges::data(self);
    }

    static constexpr auto for_each_while(auto& self, auto&& pred) -> index_t
    {
        auto iter = std::ranges::begin(self);
        auto const end = std::ranges::end(self);

        while (iter != end) {
            if (!std::invoke(pred, *iter)) {
                break;
            }
            ++iter;
        }

        return checked_cast<index_t>(iter - std::ranges::begin(self));
    }
};

} // namespace flux

#endif // FLUX_CORE_DEFAULT_IMPLS_HPP_INCLUDED


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_FUNCTIONAL_HPP_INCLUDED
#define FLUX_CORE_FUNCTIONAL_HPP_INCLUDED



#include <functional>
#include <type_traits>

namespace flux {

FLUX_EXPORT
template <typename Fn, typename Proj = std::identity>
struct proj {
    Fn fn;
    Proj prj{};

    template <typename... Args>
    constexpr auto operator()(Args&&... args)
        noexcept(noexcept(std::invoke(fn, std::invoke(prj, FLUX_FWD(args))...)))
        -> decltype(std::invoke(fn, std::invoke(prj, FLUX_FWD(args))...))
    {
        return std::invoke(fn, std::invoke(prj, FLUX_FWD(args))...);
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        noexcept(noexcept(std::invoke(fn, std::invoke(prj, FLUX_FWD(args))...)))
        -> decltype(std::invoke(fn, std::invoke(prj, FLUX_FWD(args))...))
    {
        return std::invoke(fn, std::invoke(prj, FLUX_FWD(args))...);
    }
};

template <typename F, typename P = std::identity>
proj(F, P = {}) -> proj<F, P>;

FLUX_EXPORT
template <typename Fn, typename Lhs = std::identity, typename Rhs = std::identity>
struct proj2 {
    Fn fn;
    Lhs lhs{};
    Rhs rhs{};

    template <typename Arg1, typename Arg2>
    constexpr auto operator()(Arg1&& arg1, Arg2&& arg2)
        noexcept(noexcept(std::invoke(fn, std::invoke(lhs, FLUX_FWD(arg1)),
                                          std::invoke(rhs, FLUX_FWD(arg2)))))
        -> decltype(std::invoke(fn, std::invoke(lhs, FLUX_FWD(arg1)),
                                    std::invoke(rhs, FLUX_FWD(arg2))))
    {
        return std::invoke(fn, std::invoke(lhs, FLUX_FWD(arg1)),
                           std::invoke(rhs, FLUX_FWD(arg2)));
    }

    template <typename Arg1, typename Arg2>
    constexpr auto operator()(Arg1&& arg1, Arg2&& arg2) const
        noexcept(noexcept(std::invoke(fn, std::invoke(lhs, FLUX_FWD(arg1)),
                                      std::invoke(rhs, FLUX_FWD(arg2)))))
            -> decltype(std::invoke(fn, std::invoke(lhs, FLUX_FWD(arg1)),
                                    std::invoke(rhs, FLUX_FWD(arg2))))
    {
        return std::invoke(fn, std::invoke(lhs, FLUX_FWD(arg1)),
                           std::invoke(rhs, FLUX_FWD(arg2)));
    }
};

template <typename F, typename L = std::identity, typename R = std::identity>
proj2(F, L = {}, R = {}) -> proj2<F, L, R>;

namespace detail {

template <typename Func>
struct lazy_apply {
    Func func_;

    template <typename Tuple>
    constexpr auto operator()(Tuple&& tuple) &
        noexcept(noexcept(std::apply(func_, FLUX_FWD(tuple))))
        -> decltype(std::apply(func_, FLUX_FWD(tuple)))
    {
        return std::apply(func_, FLUX_FWD(tuple));
    }

    template <typename Tuple>
    constexpr auto operator()(Tuple&& tuple) const&
        noexcept(noexcept(std::apply(func_, FLUX_FWD(tuple))))
        -> decltype(std::apply(func_, FLUX_FWD(tuple)))
    {
        return std::apply(func_, FLUX_FWD(tuple));
    }

    template <typename Tuple>
    constexpr auto operator()(Tuple&& tuple) &&
        noexcept(noexcept(std::apply(std::move(func_), FLUX_FWD(tuple))))
        -> decltype(std::apply(std::move(func_), FLUX_FWD(tuple)))
    {
        return std::apply(std::move(func_), FLUX_FWD(tuple));
    }

    template <typename Tuple>
    constexpr auto operator()(Tuple&& tuple) const&&
        noexcept(noexcept(std::apply(std::move(func_), FLUX_FWD(tuple))))
        -> decltype(std::apply(std::move(func_), FLUX_FWD(tuple)))
    {
        return std::apply(std::move(func_), FLUX_FWD(tuple));
    }
};

struct unpack_fn {
    template <typename Func>
    constexpr auto operator()(Func&& func) const
        -> lazy_apply<std::decay_t<Func>>
    {
        return lazy_apply<std::decay_t<Func>>{.func_ = FLUX_FWD(func)};
    }
};

struct flip_fn {
    template <typename Fn>
    struct flipped {
        Fn fn;

        template <typename T, typename U, typename... Args>
            requires std::invocable<Fn&, U, T, Args...>
        constexpr auto operator()(T&& t, U&& u, Args&&... args) &
            -> decltype(auto)
        {
            return std::invoke(fn, FLUX_FWD(u), FLUX_FWD(t), FLUX_FWD(args)...);
        }

        template <typename T, typename U, typename... Args>
            requires std::invocable<Fn const&, U, T, Args...>
        constexpr auto operator()(T&& t, U&& u, Args&&... args) const&
            -> decltype(auto)
        {
            return std::invoke(fn, FLUX_FWD(u), FLUX_FWD(t), FLUX_FWD(args)...);
        }

        template <typename T, typename U, typename... Args>
            requires std::invocable<Fn, U, T, Args...>
        constexpr auto operator()(T&& t, U&& u, Args&&... args) &&
            -> decltype(auto)
        {
            return std::invoke(std::move(fn), FLUX_FWD(u), FLUX_FWD(t), FLUX_FWD(args)...);
        }

        template <typename T, typename U, typename... Args>
            requires std::invocable<Fn const, U, T, Args...>
        constexpr auto operator()(T&& t, U&& u, Args&&... args) const &&
            -> decltype(auto)
        {
            return std::invoke(std::move(fn), FLUX_FWD(u), FLUX_FWD(t), FLUX_FWD(args)...);
        }
    };

    template <typename Fn>
    [[nodiscard]]
    constexpr auto operator()(Fn func) const
    {
        return flipped<Fn>{std::move(func)};
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto unpack = detail::unpack_fn{};
FLUX_EXPORT inline constexpr auto flip = detail::flip_fn{};

namespace pred {

namespace detail {

template <typename Lambda>
struct predicate : Lambda {};

template <typename L>
predicate(L) -> predicate<L>;

template <typename Op>
inline constexpr auto cmp = [](auto&& val) {
    return predicate{[val = FLUX_FWD(val)](auto const& other) {
            return Op{}(other, val);
        }};
};

} // namespace detail

/// Given a predicate, returns a new predicate with the condition reversed
FLUX_EXPORT inline constexpr auto not_ = [](auto&& pred) {
    return detail::predicate([p = FLUX_FWD(pred)] (auto const&... args) {
        return !std::invoke(p, FLUX_FWD(args)...);
    });
};

/// Returns a new predicate which is satisifed only if both the given predicates
/// return `true`.
///
/// The returned predicate is short-circuiting: if the first predicate returns
/// `false`, the second will not be evaluated.
FLUX_EXPORT inline constexpr auto both = [](auto&& p, auto&& and_) {
    return detail::predicate{[p1 = FLUX_FWD(p), p2 = FLUX_FWD(and_)] (auto const&... args) {
        return std::invoke(p1, args...) && std::invoke(p2, args...);
    }};
};

/// Returns a new predicate which is satisfied only if either of the given
/// predicates return `true`.
///
/// The returned predicate is short-circuiting: if the first predicate returns
/// `true`, the second will not be evaluated
FLUX_EXPORT inline constexpr auto either = [](auto&& p, auto&& or_) {
     return detail::predicate{[p1 = FLUX_FWD(p), p2 = FLUX_FWD(or_)] (auto const&... args) {
        return std::invoke(p1, args...) || std::invoke(p2, args...);
     }};
};

namespace detail {

FLUX_EXPORT
template <typename P>
constexpr auto operator!(detail::predicate<P> pred)
{
    return not_(std::move(pred));
}

FLUX_EXPORT
template <typename L, typename R>
constexpr auto operator&&(detail::predicate<L> lhs, detail::predicate<R> rhs)
{
    return both(std::move(lhs), std::move(rhs));
}

FLUX_EXPORT
template <typename L, typename R>
constexpr auto operator||(detail::predicate<L> lhs, detail::predicate<R> rhs)
{
    return either(std::move(lhs), std::move(rhs));
}

} // namespace detail

/// Returns a new predicate with is satified only if both of the given
/// predicates return `false`.
///
/// The returned predicate is short-circuiting: if the first predicate returns
/// `true`, the second will not be evaluated.
FLUX_EXPORT inline constexpr auto neither = [](auto&& p1, auto&& nor) {
    return not_(either(FLUX_FWD(p1), FLUX_FWD(nor)));
};

FLUX_EXPORT inline constexpr auto eq = detail::cmp<std::ranges::equal_to>;
FLUX_EXPORT inline constexpr auto neq = detail::cmp<std::ranges::not_equal_to>;
FLUX_EXPORT inline constexpr auto lt = detail::cmp<std::ranges::less>;
FLUX_EXPORT inline constexpr auto gt = detail::cmp<std::ranges::greater>;
FLUX_EXPORT inline constexpr auto leq = detail::cmp<std::ranges::less_equal>;
FLUX_EXPORT inline constexpr auto geq = detail::cmp<std::ranges::greater_equal>;

/// A predicate which always returns true
FLUX_EXPORT inline constexpr auto true_ = detail::predicate{[](auto const&...) -> bool { return true; }};

/// A predicate which always returns false
FLUX_EXPORT inline constexpr auto false_ = detail::predicate{[](auto const&...) -> bool { return false; }};

/// Identity predicate, returns the boolean value given to it
FLUX_EXPORT inline constexpr auto id = detail::predicate{[](bool b) -> bool { return b; }};

/// Returns true if the given value is greater than a zero of the same type.
FLUX_EXPORT inline constexpr auto positive = detail::predicate{[](auto const& val) -> bool {
    return val > decltype(val){0};
}};

/// Returns true if the given value is less than a zero of the same type.
FLUX_EXPORT inline constexpr auto negative = detail::predicate{[](auto const& val) -> bool {
    return val < decltype(val){0};
}};

/// Returns true if the given value is not equal to a zero of the same type.
FLUX_EXPORT inline constexpr auto nonzero = detail::predicate{[](auto const& val) -> bool {
    return val != decltype(val){0};
}};

/// Given a sequence of values, constructs a predicate which returns true
/// if its argument compares equal to one of the values
FLUX_EXPORT inline constexpr auto in = [](auto const&... vals)  requires (sizeof...(vals) > 0)
{
    return detail::predicate{[vals...](auto const& arg) -> bool {
        return ((arg == vals) || ...);
    }};
};

FLUX_EXPORT inline constexpr auto even = detail::predicate([](auto const& val) -> bool {
    return val % decltype(val){2} == decltype(val){0};
});

FLUX_EXPORT inline constexpr auto odd = detail::predicate([](auto const& val) -> bool {
  return val % decltype(val){2} != decltype(val){0};
});

} // namespace pred

namespace cmp {

namespace detail {

struct compare_floating_point_unchecked_fn {
    template <std::floating_point T>
    [[nodiscard]]
    constexpr auto operator()(T a, T b) const noexcept
        -> std::weak_ordering
    {
        return a < b ? std::weak_ordering::less
             : a > b ? std::weak_ordering::greater
                     : std::weak_ordering::equivalent;
    }
};

struct min_fn {
    template <typename T, typename U, typename Cmp = std::compare_three_way>
        requires same_decayed<T, U> &&
                 std::common_reference_with<T, U> &&
                 ordering_invocable<Cmp&, T&, U&, std::weak_ordering>
    [[nodiscard]]
    constexpr auto operator()(T&& a, U&& b, Cmp cmp = Cmp{}) const
        -> std::common_reference_t<T, U>
    {
        return std::invoke(cmp, b, a) < 0 ? FLUX_FWD(b) : FLUX_FWD(a);
    }
};

struct max_fn {
    template <typename T, typename U, typename Cmp = std::compare_three_way>
        requires same_decayed<T, U> &&
                 std::common_reference_with<T, U> &&
                 ordering_invocable<Cmp&, T&, U&, std::weak_ordering>
    [[nodiscard]]
    constexpr auto operator()(T&& a, U&& b, Cmp cmp = Cmp{}) const
        -> std::common_reference_t<T, U>
    {
        return !(std::invoke(cmp, b, a) < 0) ? FLUX_FWD(b) : FLUX_FWD(a);
    }
};

struct partial_min_fn {
    template <typename T, typename U>
        requires same_decayed<T, U> &&
                 std::common_reference_with<T, U> &&
                 ordering_invocable<std::compare_three_way&, T&, U&>
    [[nodiscard]]
    constexpr auto operator()(T&& a, U&& b) const
        -> std::common_reference_t<T, U>
    {
        return (b < a) ? FLUX_FWD(b) : FLUX_FWD(a);
    }

    template <typename T, typename U, typename Cmp>
        requires same_decayed<T, U> &&
                 std::common_reference_with<T, U> &&
                 ordering_invocable<Cmp&, T&, U&>
    [[nodiscard]]
    constexpr auto operator()(T&& a, U&& b, Cmp cmp) const
        -> std::common_reference_t<T, U>
    {
        return std::invoke(cmp, b, a) < 0 ? FLUX_FWD(b) : FLUX_FWD(a);
    }
};

struct partial_max_fn {
    template <typename T, typename U>
        requires same_decayed<T, U> &&
                 std::common_reference_with<T, U> &&
                 ordering_invocable<std::compare_three_way&, T&, U&>
    [[nodiscard]]
    constexpr auto operator()(T&& a, U&& b) const
        -> std::common_reference_t<T, U>
    {
        return !(b < a) ? FLUX_FWD(b) : FLUX_FWD(a);
    }

    template <typename T, typename U, typename Cmp>
        requires same_decayed<T, U> &&
                 std::common_reference_with<T, U> &&
                 ordering_invocable<Cmp&, T&, U&>
    [[nodiscard]]
    constexpr auto operator()(T&& a, U&& b, Cmp cmp) const
        -> std::common_reference_t<T, U>
    {
        return !(std::invoke(cmp, b, a) < 0) ? FLUX_FWD(b) : FLUX_FWD(a);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto compare = std::compare_three_way{};
FLUX_EXPORT inline constexpr auto reverse_compare = flip(compare);
FLUX_EXPORT inline constexpr auto compare_floating_point_unchecked
    = detail::compare_floating_point_unchecked_fn{};
FLUX_EXPORT inline constexpr auto min = detail::min_fn{};
FLUX_EXPORT inline constexpr auto max = detail::max_fn{};
FLUX_EXPORT inline constexpr auto partial_min = detail::partial_min_fn{};
FLUX_EXPORT inline constexpr auto partial_max = detail::partial_max_fn{};

} // namespace cmp

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_INLINE_SEQUENCE_BASE_HPP_INCLUDED
#define FLUX_CORE_INLINE_SEQUENCE_BASE_HPP_INCLUDED




// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_REQUIREMENTS_HPP_INCLUDED
#define FLUX_OP_REQUIREMENTS_HPP_INCLUDED



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
concept weak_ordering_for =
    sequence<Seq1> &&
    sequence<Seq2> &&
    ordering_invocable<Fn&, element_t<Seq1>, element_t<Seq2>, std::weak_ordering> &&
    ordering_invocable<Fn&, value_t<Seq1>&, element_t<Seq2>, std::weak_ordering> &&
    ordering_invocable<Fn&, element_t<Seq1>, value_t<Seq2>&, std::weak_ordering> &&
    ordering_invocable<Fn&, value_t<Seq1>&, value_t<Seq2>&, std::weak_ordering> &&
    ordering_invocable<Fn&, common_element_t<Seq1>, common_element_t<Seq2>, std::weak_ordering>;

} // namespace flux

#endif // FLUX_OP_REQUIREMENTS_HPP_INCLUDED


namespace flux {

FLUX_EXPORT
template <cursor Cur>
struct bounds {
    FLUX_NO_UNIQUE_ADDRESS Cur from;
    FLUX_NO_UNIQUE_ADDRESS Cur to;

    friend bool operator==(bounds const&, bounds const&) = default;
};

template <cursor Cur>
bounds(Cur, Cur) -> bounds<Cur>;

FLUX_EXPORT
template <sequence Seq>
using bounds_t = bounds<cursor_t<Seq>>;

template <typename Derived>
struct inline_sequence_base {
private:
    constexpr auto derived() -> Derived& { return static_cast<Derived&>(*this); }
    constexpr auto derived() const -> Derived const& { return static_cast<Derived const&>(*this); }

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
    constexpr auto& inc(cursor_t<D>& cur, distance_t offset) { return flux::inc(derived(), cur, offset); }

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

    [[nodiscard]]
    constexpr auto data() const requires contiguous_sequence<Derived const>
    {
        return flux::data(derived());
    }

    /// Returns the number of elements in the sequence
    [[nodiscard]]
    constexpr auto size() requires sized_sequence<Derived> { return flux::size(derived()); }

    [[nodiscard]]
    constexpr auto size() const requires sized_sequence<Derived const> { return flux::size(derived()); }

    /// Returns the number of elements in the sequence as a size_t
    [[nodiscard]]
    constexpr auto usize() requires sized_sequence<Derived> { return flux::usize(derived()); }

    [[nodiscard]]
    constexpr auto usize() const requires sized_sequence<Derived const> { return flux::usize(derived()); }

    /// Returns true if the sequence contains no elements
    [[nodiscard]]
    constexpr auto is_empty()
        requires (multipass_sequence<Derived> || sized_sequence<Derived>)
    {
        return flux::is_empty(derived());
    }

    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr auto next(cursor_t<D> cur) { return flux::next(derived(), cur); }

    template <std::same_as<Derived> D = Derived>
        requires bidirectional_sequence<Derived>
    [[nodiscard]]
    constexpr auto prev(cursor_t<D> cur) { return flux::prev(derived(), cur); }

    [[nodiscard]]
    constexpr auto front() requires multipass_sequence<Derived>
    {
        return flux::front(derived());
    }

    [[nodiscard]]
    constexpr auto front() const requires multipass_sequence<Derived const>
    {
        return flux::front(derived());
    }

    [[nodiscard]]
    constexpr auto back()
        requires bidirectional_sequence<Derived> && bounded_sequence<Derived>
    {
        return flux::back(derived());
    }

    [[nodiscard]]
    constexpr auto back() const
        requires bidirectional_sequence<Derived const> && bounded_sequence<Derived const>
    {
        return flux::back(derived());
    }

    template <typename Func, typename... Args>
        requires std::invocable<Func, Derived&, Args...>
    constexpr auto _(Func&& func, Args&&... args) & -> decltype(auto)
    {
        return std::invoke(FLUX_FWD(func), derived(), FLUX_FWD(args)...);
    }

    template <typename Func, typename... Args>
        requires std::invocable<Func, Derived const&, Args...>
    constexpr auto _(Func&& func, Args&&... args) const& -> decltype(auto)
    {
        return std::invoke(FLUX_FWD(func), derived(), FLUX_FWD(args)...);
    }

    template <typename Func, typename... Args>
        requires std::invocable<Func, Derived, Args...>
    constexpr auto _(Func&& func, Args&&... args) && -> decltype(auto)
    {
        return std::invoke(FLUX_FWD(func), std::move(derived()), FLUX_FWD(args)...);
    }

    template <typename Func, typename... Args>
        requires std::invocable<Func, Derived const, Args...>
    constexpr auto _(Func&& func, Args&&... args) const&& -> decltype(auto)
    {
        return std::invoke(FLUX_FWD(func), std::move(derived()), FLUX_FWD(args)...);
    }

    constexpr auto ref() const& requires const_iterable_sequence<Derived>;

    auto ref() const&& -> void = delete;

    constexpr auto mut_ref() &;

    /*
     * Iterator support
     */
    constexpr auto begin() &;

    constexpr auto begin() const& requires sequence<Derived const>;

    constexpr auto end() &;

    constexpr auto end() const& requires sequence<Derived const>;

    /*
     * Adaptors
     */

    template <distance_t N>
    [[nodiscard]]
    constexpr auto adjacent() && requires multipass_sequence<Derived>;

    template <typename Pred>
        requires multipass_sequence<Derived> &&
                 std::predicate<Pred&, element_t<Derived>, element_t<Derived>>
    [[nodiscard]]
    constexpr auto adjacent_filter(Pred pred) &&;

    template <distance_t N, typename Func>
        requires multipass_sequence<Derived>
    [[nodiscard]]
    constexpr auto adjacent_map(Func func) &&;

    [[nodiscard]]
    constexpr auto cache_last() &&
            requires bounded_sequence<Derived> ||
                     (multipass_sequence<Derived> && not infinite_sequence<Derived>);

    [[nodiscard]]
    constexpr auto chunk(std::integral auto chunk_sz) &&;

    template <typename Pred>
        requires multipass_sequence<Derived> &&
                 std::predicate<Pred&, element_t<Derived>, element_t<Derived>>
    [[nodiscard]]
    constexpr auto chunk_by(Pred pred) &&;

    [[nodiscard]]
    constexpr auto cursors() && requires multipass_sequence<Derived>;

    [[nodiscard]]
    constexpr auto cycle() &&
            requires infinite_sequence<Derived> || multipass_sequence<Derived>;

    [[nodiscard]]
    constexpr auto cycle(std::integral auto count) && requires multipass_sequence<Derived>;

    [[nodiscard]]
    constexpr auto dedup() &&
        requires multipass_sequence<Derived> &&
                 std::equality_comparable<element_t<Derived>>;

    [[nodiscard]]
    constexpr auto drop(std::integral auto count) &&;

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto drop_while(Pred pred) &&;

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto filter(Pred pred) &&;

    template <typename Func>
        requires std::invocable<Func&, element_t<Derived>> &&
                 detail::optional_like<std::invoke_result_t<Func&, element_t<Derived>>>
    [[nodiscard]]
    constexpr auto filter_map(Func func) &&;

    [[nodiscard]]
    constexpr auto filter_deref() && requires detail::optional_like<value_t<Derived>>;

    [[nodiscard]]
    constexpr auto flatten() && requires sequence<element_t<Derived>>;

    template <adaptable_sequence Pattern>
        requires sequence<element_t<Derived>> &&
                 multipass_sequence<Pattern> &&
                 detail::flatten_with_compatible<element_t<Derived>, Pattern>
    [[nodiscard]]
    constexpr auto flatten_with(Pattern&& pattern) &&;


    template <typename Value>
        requires sequence<element_t<Derived>> &&
                 std::constructible_from<value_t<element_t<Derived>>, Value&&>
    [[nodiscard]]
    constexpr auto flatten_with(Value value) &&;

    template <typename Func>
        requires std::invocable<Func&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto map(Func func) &&;

    template <adaptable_sequence Mask>
        requires detail::boolean_testable<element_t<Mask>>
    [[nodiscard]]
    constexpr auto mask(Mask&& mask_) &&;

    [[nodiscard]]
    constexpr auto pairwise() && requires multipass_sequence<Derived>;

    template <typename Func>
        requires multipass_sequence<Derived>
    [[nodiscard]]
    constexpr auto pairwise_map(Func func) &&;

    template <typename Func, typename Init>
        requires foldable<Derived, Func, Init>
    [[nodiscard]]
    constexpr auto prescan(Func func, Init init) &&;

    [[nodiscard]]
    constexpr auto read_only() &&;

    [[nodiscard]]
    constexpr auto reverse() &&
            requires bidirectional_sequence<Derived> && bounded_sequence<Derived>;

    template <typename D = Derived, typename Func, typename Init = value_t<D>>
        requires foldable<Derived, Func, Init>
    [[nodiscard]]
    constexpr auto scan(Func func, Init init = Init{}) &&;

    template <typename Func>
        requires foldable<Derived, Func, element_t<Derived>>
    [[nodiscard]]
    constexpr auto scan_first(Func func) &&;

    [[nodiscard]]
    constexpr auto slide(std::integral auto win_sz) && requires multipass_sequence<Derived>;

    template <typename Pattern>
        requires multipass_sequence<Derived> &&
                 multipass_sequence<Pattern> &&
                 std::equality_comparable_with<element_t<Derived>, element_t<Pattern>>
    [[nodiscard]]
    constexpr auto split(Pattern&& pattern) &&;

    template <typename Delim>
        requires multipass_sequence<Derived> &&
                 std::equality_comparable_with<element_t<Derived>, Delim const&>
    [[nodiscard]]
    constexpr auto split(Delim&& delim) &&;

    template <typename Pred>
        requires multipass_sequence<Derived> &&
                 std::predicate<Pred const&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto split(Pred pred) &&;

    template <typename Pattern>
    [[nodiscard]]
    constexpr auto split_string(Pattern&& pattern) &&;

    [[nodiscard]]
    constexpr auto stride(std::integral auto by) &&;

    [[nodiscard]]
    constexpr auto take(std::integral auto count) &&;

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto take_while(Pred pred) &&;

    /*
     * Algorithms
     */

    /// Returns `true` if every element of the sequence satisfies the predicate
    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto all(Pred pred);

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto any(Pred pred);

    template <typename Value>
        requires std::equality_comparable_with<element_t<Derived>, Value const&>
    constexpr auto contains(Value const& value) -> bool;

    /// Returns the number of elements in the sequence
    constexpr auto count();

    /// Returns the number of elements in the sequence which are equal to `value`
    template <typename Value>
        requires std::equality_comparable_with<element_t<Derived>, Value const&>
    constexpr auto count_eq(Value const& value);

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    constexpr auto count_if(Pred pred);

    template <sequence Needle, typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, element_t<Derived>, element_t<Needle>> &&
                 (multipass_sequence<Derived> || sized_sequence<Derived>) &&
                 (multipass_sequence<Needle> || sized_sequence<Needle>)
    constexpr auto ends_with(Needle&& needle, Cmp cmp = {}) -> bool;

    template <typename Value>
        requires writable_sequence_of<Derived, Value const&>
    constexpr auto fill(Value const& value) -> void;

    /// Returns a cursor pointing to the first occurrence of `value` in the sequence
    template <typename Value>
        requires std::equality_comparable_with<element_t<Derived>, Value const&>
    [[nodiscard]]
    constexpr auto find(Value const&);

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto find_if(Pred pred);

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto find_if_not(Pred pred);

    template <typename Cmp = std::compare_three_way>
        requires weak_ordering_for<Cmp, Derived>
    [[nodiscard]]
    constexpr auto find_max(Cmp cmp = Cmp{});

    template <typename Cmp = std::compare_three_way>
        requires weak_ordering_for<Cmp, Derived>
    [[nodiscard]]
    constexpr auto find_min(Cmp cmp = Cmp{});

    template <typename Cmp = std::compare_three_way>
        requires weak_ordering_for<Cmp, Derived>
    [[nodiscard]]
    constexpr auto find_minmax(Cmp cmp = Cmp{});

    template <typename D = Derived, typename Func, typename Init>
        requires foldable<Derived, Func, Init>
    [[nodiscard]]
    constexpr auto fold(Func func, Init init) -> fold_result_t<D, Func, Init>;

    template <typename D = Derived, typename Func>
        requires std::invocable<Func&, value_t<D>, element_t<D>> &&
                 std::assignable_from<value_t<D>&, std::invoke_result_t<Func&, value_t<D>, element_t<D>>>
    [[nodiscard]]
    constexpr auto fold_first(Func func);

    template <typename Func>
        requires std::invocable<Func&, element_t<Derived>>
    constexpr auto for_each(Func func) -> Func;

    template <typename Pred>
        requires std::invocable<Pred&, element_t<Derived>> &&
                 detail::boolean_testable<std::invoke_result_t<Pred&, element_t<Derived>>>
    constexpr auto for_each_while(Pred pred);

    constexpr auto inplace_reverse()
        requires bounded_sequence<Derived> &&
                 detail::element_swappable_with<Derived, Derived>;

    template <typename Cmp = std::compare_three_way>
        requires weak_ordering_for<Cmp, Derived>
    constexpr auto max(Cmp cmp = Cmp{});

    template <typename Cmp = std::compare_three_way>
        requires weak_ordering_for<Cmp, Derived>
    constexpr auto min(Cmp cmp = Cmp{});

    template <typename Cmp = std::compare_three_way>
        requires weak_ordering_for<Cmp, Derived>
    constexpr auto minmax(Cmp cmp = Cmp{});

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto none(Pred pred);

    template <typename Iter>
        requires std::weakly_incrementable<Iter> &&
                 std::indirectly_writable<Iter, element_t<Derived>>
    constexpr auto output_to(Iter iter) -> Iter;

    constexpr auto sum()
        requires foldable<Derived, std::plus<>, value_t<Derived>> &&
                 std::default_initializable<value_t<Derived>>;

    template <typename Cmp = std::compare_three_way>
        requires random_access_sequence<Derived> &&
                 bounded_sequence<Derived> &&
                 detail::element_swappable_with<Derived, Derived> &&
                 weak_ordering_for<Cmp, Derived>
    constexpr void sort(Cmp cmp = {});

    constexpr auto product()
        requires foldable<Derived, std::multiplies<>, value_t<Derived>> &&
                 requires { value_t<Derived>(1); };

    template <sequence Needle, typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, element_t<Derived>, element_t<Needle>>
    constexpr auto starts_with(Needle&& needle, Cmp cmp = Cmp{}) -> bool;

    template <typename Container, typename... Args>
    constexpr auto to(Args&&... args) -> Container;

    template <template <typename...> typename Container, typename... Args>
    constexpr auto to(Args&&... args);

    auto write_to(std::ostream& os) -> std::ostream&;
};


} // namespace flux

#endif // FLUX_CORE_SEQUENCE_IFACE_HPP_INCLUDED




#endif // FLUX_CORE_HPP_INCLUDED



// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ADJACENT_HPP_INCLUDED
#define FLUX_OP_ADJACENT_HPP_INCLUDED



// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_BEGIN_END_HPP_INCLUDED
#define FLUX_OP_BEGIN_END_HPP_INCLUDED



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

template <sequence S>
struct sequence_iterator {
private:
    S* seq_ = nullptr;
    cursor_t<S> cur_{};

    template <sequence SS>
    friend struct sequence_iterator;

public:
    using value_type = value_t<S>;
    using difference_type = distance_t;
    using element_type = value_t<S>; // Yes, really
    using iterator_concept = decltype(get_iterator_tag<S>());

    sequence_iterator() requires std::default_initializable<cursor_t<S>> = default;

    constexpr sequence_iterator(S& base, cursor_t<S> cur)
        : seq_(std::addressof(base)),
          cur_(std::move(cur))
    {}

    template <typename SS = S>
        requires std::is_const_v<SS>
    constexpr sequence_iterator(sequence_iterator<std::remove_const_t<SS>> other)
        : seq_(other.seq_),
          cur_(std::move(other.cur_))
    {}

    constexpr auto operator*() const -> element_t<S>
    {
        return flux::read_at(*seq_, cur_);
    }

    constexpr auto operator++() -> sequence_iterator&
    {
        flux::inc(*seq_, cur_);
        return *this;
    }

    constexpr void operator++(int) { flux::inc(*seq_, cur_); }

    constexpr auto operator++(int) -> sequence_iterator
        requires multipass_sequence<S>
    {
        auto temp = *this;
        ++*this;
        return temp;
    }

    constexpr auto operator--() -> sequence_iterator&
        requires bidirectional_sequence<S>
    {
        flux::dec(*seq_, cur_);
        return *this;
    }

    constexpr auto operator--(int) -> sequence_iterator
        requires bidirectional_sequence<S>
    {
        auto temp = *this;
        --*this;
        return temp;
    }

    constexpr auto operator+=(difference_type n) -> sequence_iterator&
        requires random_access_sequence<S>
    {
        flux::inc(*seq_, cur_, n);
        return *this;
    }

    constexpr auto operator-=(difference_type n) -> sequence_iterator&
        requires random_access_sequence<S>
    {
        flux::inc(*seq_, cur_, -n);
        return *this;
    }

    constexpr auto operator[](difference_type n) const -> element_t<S>
    requires random_access_sequence<S>
    {
        auto i = flux::first(*seq_);
        flux::inc(*seq_, i, n);
        return flux::read_at(*seq_, i);
    }

    constexpr auto operator->() const -> std::add_pointer_t<element_t<S>>
        requires contiguous_sequence<S>
    {
        return flux::data(*seq_) + flux::distance(*seq_, flux::first(*seq_), cur_);
    }

    friend constexpr bool operator==(sequence_iterator const& self, std::default_sentinel_t)
    {
        return flux::is_last(*self.seq_, self.cur_);
    }

    friend bool operator==(sequence_iterator const&, sequence_iterator const&)
        requires multipass_sequence<S>
        = default;

    friend std::strong_ordering operator<=>(sequence_iterator const&, sequence_iterator const&)
        requires random_access_sequence<S>
        = default;

    friend constexpr auto operator+(sequence_iterator self, difference_type n)
        -> sequence_iterator
        requires random_access_sequence<S>
    {
        flux::inc(*self.seq_, self.cur_, n);
        return self;
    }

    friend constexpr auto operator+(difference_type n, sequence_iterator self)
        -> sequence_iterator
        requires random_access_sequence<S>
    {
        flux::inc(*self.seq_, self.cur_, n);
        return self;
    }

    friend constexpr auto operator-(sequence_iterator self, difference_type n)
        -> sequence_iterator
        requires random_access_sequence<S>
    {
        flux::inc(*self.seq_, self.cur_, -n);
        return self;
    }

    friend constexpr auto operator-(sequence_iterator const& lhs, sequence_iterator const& rhs)
        -> difference_type
        requires random_access_sequence<S>
    {
        FLUX_ASSERT(lhs.seq_ == rhs.seq_);
        return flux::distance(*lhs.seq_, rhs.cur_, lhs.cur_);
    }

    friend constexpr auto iter_move(sequence_iterator const& self)
        -> rvalue_element_t<S>
    {
        return flux::move_at(*self.seq_, self.cur_);
    }

    friend constexpr void iter_swap(sequence_iterator const& lhs, sequence_iterator const& rhs)
        requires element_swappable_with<S, S>
    {
        flux::swap_with(*lhs.seq_, lhs.cur_, *rhs.seq_, rhs.cur_);
    }
};

struct begin_fn {
    template <sequence S>
    constexpr auto operator()(S& seq) const
    {
        return sequence_iterator<S>(seq, flux::first(seq));
    }
};

struct end_fn {
    template <sequence S>
    constexpr auto operator()(S& seq) const
    {
        // Ranges requires sentinels to be copy-constructible
        if constexpr (bounded_sequence<S> && std::copy_constructible<cursor_t<S>>) {
            return sequence_iterator(seq, flux::last(seq));
        } else {
            return std::default_sentinel;
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto begin = detail::begin_fn{};
FLUX_EXPORT inline constexpr auto end = detail::end_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::begin() &
{
    return flux::begin(derived());
}

template <typename D>
constexpr auto inline_sequence_base<D>::begin() const&
    requires sequence<D const>
{
    return flux::begin(derived());
};

template <typename D>
constexpr auto inline_sequence_base<D>::end() &
{
    return flux::end(derived());
}

template <typename D>
constexpr auto inline_sequence_base<D>::end() const&
requires sequence<D const>
{
    return flux::end(derived());
};

} // namespace flux

// Every sequence is a range: furthermore, it is a view if it is either
// trivially copyable, or not copyable at all
// See P2415 for the logic behind this
template <flux::detail::derived_from_inline_sequence_base Seq>
inline constexpr bool std::ranges::enable_view<Seq> =
    std::is_trivially_copyable_v<Seq> || !std::copyable<Seq>;

#endif // FLUX_OP_BEGIN_END_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_REVERSE_HPP_INCLUDED
#define FLUX_OP_REVERSE_HPP_INCLUDED



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
        return traits_t<Seq>::for_each_while(seq, std::move(pred));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto for_each_while = detail::for_each_while_fn{};

template <typename Derived>
template <typename Pred>
    requires std::invocable<Pred&, element_t<Derived>> &&
             detail::boolean_testable<std::invoke_result_t<Pred&, element_t<Derived>>>
constexpr auto inline_sequence_base<Derived>::for_each_while(Pred pred)
{
    return flux::for_each_while(derived(), std::ref(pred));
}

} // namespace flux

#endif // FLUX_OP_FOR_EACH_WHILE_HPP_INCLUDED


namespace flux {

namespace detail {

template <typename Base>
struct passthrough_traits_base : default_sequence_traits {

    static constexpr auto first(auto& self)
        -> decltype(flux::first(self.base()))
    {
        return flux::first(self.base());
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, auto const& cur)
        -> decltype(flux::is_last(self.base(), cur))
    {
        return flux::is_last(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, auto const& cur)
        -> decltype(flux::read_at(self.base(), cur))
    {
        return flux::read_at(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, auto& cur)
        -> decltype(flux::inc(self.base(), cur))
    {
        return flux::inc(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto dec(Self& self, auto& cur)
        -> decltype(flux::dec(self.base(), cur))
    {
        return flux::dec(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, auto& cur, distance_t dist)
        -> decltype(flux::inc(self.base(), cur, dist))
    {
        return flux::inc(self.base(), cur, dist);
    }

    template <typename Self>
    static constexpr auto distance(Self& self, auto const& from, auto const& to)
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
    static constexpr auto size(Self& self) -> decltype(flux::size(self.base()))
    {
        return flux::size(self.base());
    }

    static constexpr auto last(auto& self) -> decltype(flux::last(self.base()))
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
    static constexpr auto read_at_unchecked(Self& self, auto const& cur)
        -> decltype(flux::read_at_unchecked(self.base(), cur))
    {
        return flux::read_at_unchecked(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto move_at_unchecked(Self& self, auto const& cur)
        -> decltype(flux::move_at_unchecked(self.base(), cur))
    {
        return flux::move_at_unchecked(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto for_each_while(Self& self, auto&& pred)
        -> decltype(flux::for_each_while(self.base(), FLUX_FWD(pred)))
    {
        return flux::for_each_while(self.base(), FLUX_FWD(pred));
    }
};

template <sequence Base>
struct ref_adaptor : inline_sequence_base<ref_adaptor<Base>> {
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

    // We are always movable
    ref_adaptor(ref_adaptor&&) = default;
    ref_adaptor& operator=(ref_adaptor&&) = default;
    ~ref_adaptor() = default;

    // ...but only copyable when `Base` is const
    ref_adaptor(ref_adaptor const&) requires std::is_const_v<Base> = default;
    ref_adaptor& operator=(ref_adaptor const&) requires std::is_const_v<Base> = default;

    constexpr Base& base() const noexcept { return *base_; }

    struct flux_sequence_traits : passthrough_traits_base<Base> {
        using value_type = value_t<Base>;
    };
};

template <typename>
inline constexpr bool is_ref_adaptor = false;

template <typename T>
inline constexpr bool is_ref_adaptor<ref_adaptor<T>> = true;

template <sequence Base>
    requires std::movable<Base>
struct owning_adaptor : inline_sequence_base<owning_adaptor<Base>> {
private:
    Base base_;

public:
    constexpr explicit owning_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    constexpr Base& base() & noexcept { return base_; }
    constexpr Base const& base() const& noexcept { return base_; }
    constexpr Base&& base() && noexcept { return std::move(base_); }
    constexpr Base const&& base() const&& noexcept { return std::move(base_); }

    struct flux_sequence_traits : passthrough_traits_base<Base> {
        using value_type = value_t<Base>;
    };
};

struct mut_ref_fn {
    template <sequence Seq>
        requires (!std::is_const_v<Seq>)
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

struct ref_fn {
    template <const_iterable_sequence Seq>
        requires (!is_ref_adaptor<Seq>)
    [[nodiscard]]
    constexpr auto operator()(Seq const& seq) const
    {
        return ref_adaptor<Seq const>(seq);
    }

    template <const_iterable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(ref_adaptor<Seq> ref) const
    {
        return ref_adaptor<Seq const>(ref.base());
    }

    template <typename T>
    auto operator()(T const&&) const -> void = delete;
};

} // namespace detail

FLUX_EXPORT inline constexpr auto mut_ref = detail::mut_ref_fn{};
FLUX_EXPORT inline constexpr auto ref = detail::ref_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::ref() const&
    requires const_iterable_sequence<D>
{
    return flux::ref(derived());
}

template <typename D>
constexpr auto inline_sequence_base<D>::mut_ref() &
{
    return flux::mut_ref(derived());
}

} // namespace flux

#endif // FLUX_OP_REF_HPP_INCLUDED


namespace flux {

namespace detail {

struct from_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr (derived_from_inline_sequence_base<Seq>) {
            return FLUX_FWD(seq);
        } else {
            return owning_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
        }
    }
};

struct from_fwd_ref_fn {
    template <sequence Seq>
        requires adaptable_sequence<Seq> || std::is_lvalue_reference_v<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr (std::is_lvalue_reference_v<Seq>) {
            if constexpr (std::is_const_v<std::remove_reference_t<Seq>>) {
                return flux::ref(seq);
            } else {
                return flux::mut_ref(seq);
            }
        } else {
            return from_fn{}(seq);
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto from = detail::from_fn{};
FLUX_EXPORT inline constexpr auto from_fwd_ref = detail::from_fwd_ref_fn{};

} // namespace flux

#endif // FLUX_OP_FROM_HPP_INCLUDED


namespace flux {

namespace detail {

template <bidirectional_sequence Base>
    requires bounded_sequence<Base>
struct reverse_adaptor : inline_sequence_base<reverse_adaptor<Base>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;

public:
    constexpr explicit reverse_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base&& { return std::move(base_); }

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;

            friend bool operator==(cursor_type const&, cursor_type const&)
                requires std::equality_comparable<cursor_t<Base>>
            = default;

            friend auto operator<=>(cursor_type const& lhs, cursor_type const& rhs)
                -> std::strong_ordering
                requires std::three_way_comparable<cursor_t<Base>, std::strong_ordering>
            {
                return rhs <=> lhs;
            }
        };

    public:
        using value_type = value_t<Base>;

        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type(flux::last(self.base_));
        }

        static constexpr auto last(auto& self) -> cursor_type
        {
            return cursor_type(flux::first(self.base_));
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return cur.base_cur == flux::first(self.base_);
        }

        template <typename Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur)
            -> element_t<decltype((self.base_))>
        {
            return flux::read_at(self.base_, flux::prev(self.base_, cur.base_cur));
        }

        template <typename Self>
        static constexpr auto read_at_unchecked(Self& self, cursor_type const& cur)
            -> element_t<decltype((self.base_))>
        {
            return flux::read_at_unchecked(self.base_, flux::prev(self.base_, cur.base_cur));
        }

        template <typename Self>
        static constexpr auto move_at(Self& self, cursor_type const& cur)
            -> rvalue_element_t<decltype((self.base_))>
        {
            return flux::move_at(self.base_, flux::prev(self.base_, cur.base_cur));
        }

        template <typename Self>
        static constexpr auto move_at_unchecked(Self& self, cursor_type const& cur)
            -> rvalue_element_t<decltype((self.base_))>
        {
            return flux::move_at_unchecked(self.base_, flux::prev(self.base_, cur.base_cur));
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            flux::dec(self.base_, cur.base_cur);
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
        {
            flux::inc(self.base_, cur.base_cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur, distance_t dist) -> void
        {
            flux::inc(self.base_, cur.base_cur, num::checked_sub(distance_t{}, dist));
        }

        static constexpr auto distance(auto& self, cursor_type const& from, cursor_type const& to)
            -> distance_t
        {
            return flux::distance(self.base_, to.base_cur, from.base_cur);
        }

        static constexpr auto size(auto& self) -> distance_t
            requires sized_sequence<decltype((self.base_))>
        {
            return flux::size(self.base_);
        }

        static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_type
            requires bidirectional_sequence<decltype((self.base_))> &&
                     bounded_sequence<decltype((self.base_))>
        {
            auto cur = flux::last(self.base_);
            const auto end = flux::first(self.base_);

            while (cur != end) {
                flux::dec(self.base_, cur);
                if (!std::invoke(pred, flux::read_at(self.base_, cur))) {
                    flux::inc(self.base_, cur);
                    break;
                }
            }

            return cursor_type(cur);
        }
    };
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
        -> sequence auto
    {
        if constexpr (is_reverse_adaptor<std::decay_t<Seq>>) {
            return FLUX_FWD(seq).base();
        } else {
            return reverse_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto reverse = detail::reverse_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::reverse() &&
    requires bidirectional_sequence<D> && bounded_sequence<D>
{
    return flux::reverse(std::move(derived()));
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



namespace flux {

namespace detail {

template <typename T>
struct empty_sequence : inline_sequence_base<empty_sequence<T>> {
    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            friend auto operator==(cursor_type, cursor_type) -> bool = default;
            friend auto operator<=>(cursor_type, cursor_type) = default;
        };

    public:
        static constexpr auto first(empty_sequence) -> cursor_type { return {}; }
        static constexpr auto last(empty_sequence) -> cursor_type { return {}; }
        static constexpr auto is_last(empty_sequence, cursor_type) -> bool { return true; }

        static constexpr auto inc(empty_sequence, cursor_type& cur, distance_t = 0)
            -> cursor_type&
        {
            return cur;
        }

        static constexpr auto dec(empty_sequence, cursor_type& cur) -> cursor_type&
        {
            return cur;
        }

        static constexpr auto distance(empty_sequence, cursor_type, cursor_type)
            -> std::ptrdiff_t
        {
            return 0;
        }

        static constexpr auto size(empty_sequence) -> std::ptrdiff_t { return 0; }
        static constexpr auto data(empty_sequence) -> std::add_pointer_t<T> requires std::is_object_v<T> { return nullptr; }

        [[noreturn]]
        static constexpr auto read_at(empty_sequence, cursor_type) -> T&
        {
            runtime_error("Attempted read of flux::empty");
        }
    };
};

} // namespace detail

FLUX_EXPORT
template <typename T>
inline constexpr auto empty = detail::empty_sequence<T>{};

} // namespace flux

#endif // FLUX_SOURCE_EMPTY_HPP_INCLUDED


#include <algorithm> // for std::min({ilist...})

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

}

template <typename... Bases>
struct zip_traits_base : default_sequence_traits {
private:
    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

protected:

    template <typename... Ts>
    using tuple_t = detail::pair_or_tuple_t<Ts...>;

    template <std::size_t I>
    static constexpr decltype(auto) read1_(auto fn, auto& self, auto const& cur)
    {
        return fn(std::get<I>(self.bases_), std::get<I>(cur));
    }

public:

    static constexpr bool is_infinite = (infinite_sequence<Bases> && ...);

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto first(Self& self)
    {
        return std::apply([](auto&&... args) {
            return tuple_t<decltype(flux::first(FLUX_FWD(args)))...>(flux::first(FLUX_FWD(args))...);
        }, self.bases_);
    }

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr bool is_last(Self& self, cursor_t<Self> const& cur)
    {
        return [&self, &cur]<std::size_t... I>(std::index_sequence<I...>) {
            return (flux::is_last(std::get<I>(self.bases_), std::get<I>(cur)) || ...);
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto& inc(Self& self, cursor_t<Self>& cur)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::inc(std::get<I>(self.bases_), std::get<I>(cur)), ...);
        }(std::index_sequence_for<Bases...>{});

        return cur;
    }

    template <typename Self>
        requires (bidirectional_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto& dec(Self& self, cursor_t<Self>& cur)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::dec(std::get<I>(self.bases_), std::get<I>(cur)), ...);
        }(std::index_sequence_for<Bases...>{});

        return cur;
    }

    template <typename Self>
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto& inc(Self& self, cursor_t<Self>& cur, distance_t offset)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (flux::inc(std::get<I>(self.bases_), std::get<I>(cur), offset), ...);
        }(std::index_sequence_for<Bases...>{});

        return cur;
    }

    template <typename Self>
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto distance(Self& self, cursor_t<Self> const& from,
                                   cursor_t<Self> const& to)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return std::min({flux::distance(std::get<I>(self.bases_), std::get<I>(from), std::get<I>(to))...});
        }(std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...)
                && (sized_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto last(Self& self)
    {
        auto cur = first(self);
        return inc(self, cur, size(self));
    }

    template <typename Self>
        requires (sized_sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto size(Self& self)
    {
        return std::apply([&](auto&... args) {
            return std::min({flux::size(args)...});
        }, self.bases_);
    }
};


namespace detail {

template <sequence... Bases>
struct zip_adaptor : inline_sequence_base<zip_adaptor<Bases...>> {
private:
    pair_or_tuple_t<Bases...> bases_;

    friend struct sequence_traits<zip_adaptor>;
    friend struct zip_traits_base<Bases...>;

public:
    constexpr explicit zip_adaptor(decays_to<Bases> auto&&... bases)
        : bases_(FLUX_FWD(bases)...)
    {}
};

struct zip_fn {
    template <adaptable_sequence... Seqs>
    [[nodiscard]]
    constexpr auto operator()(Seqs&&... seqs) const
    {
        if constexpr (sizeof...(Seqs) == 0) {
            return empty<std::tuple<>>;
        } else {
            return zip_adaptor<std::decay_t<Seqs>...>(FLUX_FWD(seqs)...);
        }
    }
};

template <typename Func, sequence... Bases>
struct zip_map_adaptor : inline_sequence_base<zip_map_adaptor<Func, Bases...>> {
private:
    pair_or_tuple_t<Bases...> bases_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

    friend struct sequence_traits<zip_map_adaptor>;
    friend struct zip_traits_base<Bases...>;

public:
    constexpr explicit zip_map_adaptor(Func&& func, decays_to<Bases> auto&&... bases)
        : bases_(FLUX_FWD(bases)...), func_(std::move(func))
    {}
};

struct zip_map_fn {
    template <typename Func, adaptable_sequence... Seqs>
        requires std::regular_invocable<Func&, element_t<Seqs>...>
    [[nodiscard]]
    constexpr auto operator()(Func func, Seqs&&... seqs) const
    {
        if constexpr (sizeof...(Seqs) == 0) {
            return empty<std::invoke_result_t<Func>>;
        } else {
            return zip_map_adaptor<Func, std::decay_t<Seqs>...>(std::move(func), FLUX_FWD(seqs)...);
        }
    }
};

} // namespace detail

template <typename... Bases>
struct sequence_traits<detail::zip_adaptor<Bases...>> : zip_traits_base<Bases...>
{
private:
    using base = zip_traits_base<Bases...>;

    template <typename... Ts>
    using tuple_t = base::template tuple_t<Ts...>;

    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    static constexpr auto read_(auto fn, auto& self, auto const& cur)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
          return tuple_t<decltype(base::template read1_<I>(fn, self, cur))...> {
              base::template read1_<I>(fn, self, cur)...
          };
        }(std::index_sequence_for<Bases...>{});
    }

public:
    using value_type = tuple_t<value_t<Bases>...>;

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::read_at, self, cur);
    }

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::move_at, self, cur);
    }

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto read_at_unchecked(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::read_at_unchecked, self, cur);
    }

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr auto move_at_unchecked(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::move_at_unchecked, self, cur);
    }

};

template <typename Func, typename... Bases>
struct sequence_traits<detail::zip_map_adaptor<Func, Bases...>> : zip_traits_base<Bases...>
{
private:
    using base = zip_traits_base<Bases...>;

    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    static constexpr decltype(auto) read_(auto fn, auto& self, auto const& cur)
    {
        return [&]<std::size_t... I>(std::index_sequence<I...>) -> decltype(auto) {
            return std::invoke(self.func_,
                base::template read1_<I>(fn, self, cur)...
            );
        }(std::index_sequence_for<Bases...>{});
    }

public:
    using value_type = std::remove_cvref_t<std::invoke_result_t<Func&, element_t<Bases>...>>;

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr decltype(auto) read_at(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::read_at, self, cur);
    }

    template <typename Self>
        requires (sequence<const_like_t<Self, Bases>> && ...)
    static constexpr decltype(auto) read_at_unchecked(Self& self, cursor_t<Self> const& cur)
    {
        return read_(flux::read_at_unchecked, self, cur);
    }

    using default_sequence_traits::move_at;
    using default_sequence_traits::move_at_unchecked;
};

FLUX_EXPORT inline constexpr auto zip = detail::zip_fn{};
FLUX_EXPORT inline constexpr auto zip_map = detail::zip_map_fn{};

} // namespace flux

#endif // FLUX_OP_ZIP_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_IOTA_HPP_INCLUDED
#define FLUX_SOURCE_IOTA_HPP_INCLUDED



namespace flux {

namespace detail {

// These concepts mirror the standard ones, except that iter_difference_t is not required
template <typename T>
concept incrementable =
    std::regular<T> &&
    requires (T t) {
        { ++t } -> std::same_as<T&>;
        { t++ } -> std::same_as<T>;
    };

template <typename T>
concept decrementable =
    incrementable<T> &&
    requires (T t) {
        { --t } -> std::same_as<T&>;
        { t-- } -> std::same_as<T>;
    };

template <typename T>
concept advancable =
    decrementable<T> &&
    std::totally_ordered<T> &&
    std::weakly_incrementable<T> && // iter_difference_t exists
    requires (T t, T const u, std::iter_difference_t<T> o) {
        { t += o } -> std::same_as<T&>;
        { t -= o } -> std::same_as<T&>;
        T(u + o);
        T(o + u);
        T(u - o);
        { u - u } -> std::convertible_to<distance_t>;
    };

struct iota_traits {
    bool has_start;
    bool has_end;
};

template <incrementable T, iota_traits Traits>
struct iota_sequence_traits : default_sequence_traits {
    using cursor_type = T;

    static constexpr bool is_infinite = !Traits.has_end;

    static constexpr auto first(auto& self) -> cursor_type
    {
        if constexpr (Traits.has_start) {
            return self.start_;
        } else {
            return cursor_type{};
        }
    }

    static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
    {
        if constexpr (Traits.has_end) {
            return cur == self.end_;
        } else {
            return false;
        }
    }

    static constexpr auto inc(auto&, cursor_type& cur) -> cursor_type&
    {
        return ++cur;
    }

    static constexpr auto read_at(auto&, cursor_type const& cur) -> T
    {
        return cur;
    }

    static constexpr auto last(auto& self) -> cursor_type
        requires (Traits.has_end)
    {
        return self.end_;
    }

    static constexpr auto dec(auto&, cursor_type& cur) -> cursor_type&
        requires decrementable<T>
    {
        return --cur;
    }

    static constexpr auto inc(auto&, cursor_type& cur, distance_t offset)
        -> cursor_type&
        requires advancable<T>
    {
        return cur += checked_cast<std::iter_difference_t<T>>(offset);
    }

    static constexpr auto distance(auto&, cursor_type const& from, cursor_type const& to)
        requires advancable<T>
    {
        return from <= to ? checked_cast<distance_t>(to - from) : -checked_cast<distance_t>(from - to);
    }

    static constexpr auto size(auto& self) -> distance_t
        requires advancable<T> && (Traits.has_start && Traits.has_end)
    {
        return checked_cast<distance_t>(self.end_ - self.start_);
    }
};

template <typename T>
struct basic_iota_sequence : inline_sequence_base<basic_iota_sequence<T>> {
    using flux_sequence_traits = iota_sequence_traits<T, iota_traits{}>;
    friend flux_sequence_traits;
};

template <typename T>
struct iota_sequence : inline_sequence_base<iota_sequence<T>> {
private:
    T start_;

    static constexpr iota_traits traits{.has_start = true, .has_end = false};

public:
    inline constexpr explicit iota_sequence(T from)
        : start_(std::move(from))
    {}

    using flux_sequence_traits = iota_sequence_traits<T, traits>;
    friend flux_sequence_traits;
};

template <typename T>
struct bounded_iota_sequence : inline_sequence_base<bounded_iota_sequence<T>> {
    T start_;
    T end_;

    static constexpr iota_traits traits{.has_start = true, .has_end = true};

public:
    inline constexpr bounded_iota_sequence(T from, T to)
        : start_(std::move(from)),
          end_(std::move(to))
    {}

    using flux_sequence_traits = iota_sequence_traits<T, traits>;
    friend flux_sequence_traits;
};

struct iota_fn {
    template <incrementable T>
    constexpr auto operator()(T from) const
    {
        return iota_sequence<T>(std::move(from));
    }

    template <incrementable T>
    constexpr auto operator()(T from, T to) const
    {
        return bounded_iota_sequence<T>(std::move(from), std::move(to));
    }
};

struct ints_fn {
    inline constexpr auto operator()() const
    {
        return basic_iota_sequence<distance_t>();
    }

    inline constexpr auto operator()(distance_t from) const
    {
        return iota_sequence<distance_t>(from);
    }

    inline constexpr auto operator()(distance_t from, distance_t to) const
    {
        return bounded_iota_sequence<distance_t>(from, to);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto iota = detail::iota_fn{};
FLUX_EXPORT inline constexpr auto ints = detail::ints_fn{};

} // namespace flux

#endif // FLUX_SOURCE_IOTA_HPP_INCLUDED


#include <array>


namespace flux {

namespace detail {

template <typename Base, distance_t N>
struct adjacent_sequence_traits_base : default_sequence_traits {
protected:
    struct cursor_type {
        std::array<cursor_t<Base>, N> arr{};

        friend constexpr auto operator==(cursor_type const& lhs, cursor_type const& rhs)
            -> bool
        {
            return lhs.arr.back() == rhs.arr.back();
        }

        friend constexpr auto operator<=>(cursor_type const& lhs, cursor_type const& rhs)
            -> std::strong_ordering
            requires ordered_cursor<cursor_t<Base>>
        {
            return lhs.arr.back() <=> rhs.arr.back();
        }
    };

public:

    static inline constexpr bool is_infinite = infinite_sequence<Base>;

    static constexpr auto first(auto& self) -> cursor_type
    {
        cursor_type out{flux::first(self.base_), };

        FLUX_FOR(auto i, flux::iota(std::size_t{1}, std::size_t{N})) {
            out.arr[i] = out.arr[i - 1];
            if (!flux::is_last(self.base_, out.arr[i])) {
                flux::inc(self.base_, out.arr[i]);
            }
        }
        return out;
    }

    static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
    {
        return flux::is_last(self.base_, cur.arr.back());
    }

    static constexpr auto inc(auto& self, cursor_type& cur) -> void
    {
        std::apply([&](auto&... curs) {
            (flux::inc(self.base_, curs), ...);
        }, cur.arr);
    }

    static constexpr auto last(auto& self) -> cursor_type
        requires (bidirectional_sequence<Base> && bounded_sequence<Base>)
    {
        cursor_type out{};
        out.arr.back() = flux::last(self.base_);
        auto const first = flux::first(self.base_);
        FLUX_FOR(auto i, flux::iota(std::size_t{0}, std::size_t{N}-1).reverse()) {
            out.arr[i] = out.arr[i + 1];
            if (out.arr[i] != first) {
                flux::dec(self.base_, out.arr[i]);
            }
        }
        return out;
    }

    static constexpr auto dec(auto& self, cursor_type& cur) -> void
        requires bidirectional_sequence<Base>
    {
        std::apply([&self](auto&... curs) {
            (flux::dec(self.base_, curs), ...);
        }, cur.arr);
    }

    static constexpr auto inc(auto& self, cursor_type& cur, distance_t offset) -> void
        requires random_access_sequence<Base>
    {
        std::apply([&self, offset](auto&... curs) {
            (flux::inc(self.base_, curs, offset), ...);
        }, cur.arr);
    }

    static constexpr auto distance(auto& self, cursor_type const& from, cursor_type const& to)
        -> distance_t
        requires random_access_sequence<Base>
    {
        return flux::distance(self.base_, from.arr.back(), to.arr.back());
    }

    static constexpr auto size(auto& self) -> distance_t
        requires sized_sequence<Base>
    {
        auto s = (flux::size(self.base_) - N) + 1;
        return (std::max)(s, distance_t{0});
    }
};

template <typename Base, distance_t N>
struct adjacent_adaptor : inline_sequence_base<adjacent_adaptor<Base, N>> {
private:
    Base base_;

    friend struct adjacent_sequence_traits_base<Base, N>;

public:
    constexpr explicit adjacent_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_sequence_traits : adjacent_sequence_traits_base<Base, N> {
    private:

        using cursor_type = adjacent_sequence_traits_base<Base, N>::cursor_type;

        template <auto& ReadFn>
        static constexpr auto do_read(auto& self, cursor_type const& cur)
        {
            return std::apply([&](auto const&... curs) {
                return pair_or_tuple_t<decltype(ReadFn(self.base_, curs))...>(
                    ReadFn(self.base_, curs)...);
            }, cur.arr);
        }

        template <std::size_t I>
        using base_value_t = value_t<Base>;

        template <std::size_t... Is>
        static auto make_value_type(std::index_sequence<Is...>) -> pair_or_tuple_t<base_value_t<Is>...>;

    public:
        using value_type = decltype(make_value_type(std::make_index_sequence<N>{}));

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(do_read<flux::read_at>(self, cur))
        {
            return do_read<flux::read_at>(self, cur);
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(do_read<flux::move_at>( self, cur))
        {
            return do_read<flux::move_at>( self, cur);
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(do_read<flux::read_at_unchecked>(self, cur))
        {
            return do_read<flux::read_at_unchecked>(self, cur);
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(do_read<flux::move_at_unchecked>(self, cur))
        {
            return do_read<flux::move_at_unchecked>(self, cur);
        }
    };
};

template <typename Base, distance_t N, typename Func>
struct adjacent_map_adaptor : inline_sequence_base<adjacent_map_adaptor<Base, N, Func>> {
private:
    Base base_;
    Func func_;

    friend struct adjacent_sequence_traits_base<Base, N>;

public:
    constexpr explicit adjacent_map_adaptor(decays_to<Base> auto&& base, Func&& func)
        : base_(FLUX_FWD(base)),
          func_(std::move(func))
    {}

    struct flux_sequence_traits : adjacent_sequence_traits_base<Base, N> {
        template <typename Self>
        static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
            -> decltype(auto)
            requires repeated_invocable<decltype((self.func_)), element_t<decltype((self.base_))>, N>
        {
            return std::apply([&](auto const&... curs) {
                return std::invoke(self.func_, flux::read_at(self.base_, curs)...);
            }, cur.arr);
        }
    };
};

template <distance_t N>
struct adjacent_fn {
    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> multipass_sequence auto
    {
        return adjacent_adaptor<std::decay_t<Seq>, N>(FLUX_FWD(seq));
    }
};

template <distance_t N>
struct adjacent_map_fn {
    template <adaptable_sequence Seq, typename Func>
        requires multipass_sequence<Seq> &&
                 repeated_invocable<Func, element_t<Seq>, N>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func) const -> multipass_sequence auto
    {
        return adjacent_map_adaptor<std::decay_t<Seq>, N, Func>(
            FLUX_FWD(seq), std::move(func));
    }
};

} // namespace detail

FLUX_EXPORT
template <distance_t N>
    requires (N > 0)
inline constexpr auto adjacent = detail::adjacent_fn<N>{};

FLUX_EXPORT inline constexpr auto pairwise = adjacent<2>;

FLUX_EXPORT
template <distance_t N>
    requires (N > 0)
inline constexpr auto adjacent_map = detail::adjacent_map_fn<N>{};

FLUX_EXPORT inline constexpr auto pairwise_map = adjacent_map<2>;

template <typename D>
template <distance_t N>
constexpr auto inline_sequence_base<D>::adjacent() &&
    requires multipass_sequence<D>
{
    return flux::adjacent<N>(std::move(derived()));
}

template <typename D>
constexpr auto inline_sequence_base<D>::pairwise() &&
    requires multipass_sequence<D>
{
    return flux::pairwise(std::move(derived()));
}

template <typename D>
template <distance_t N, typename Func>
    requires multipass_sequence<D>
constexpr auto inline_sequence_base<D>::adjacent_map(Func func) &&
{
    return flux::adjacent_map<N>(std::move(derived()), std::move(func));
}

template <typename D>
template <typename Func>
    requires multipass_sequence<D>
constexpr auto inline_sequence_base<D>::pairwise_map(Func func) &&
{
    return flux::pairwise_map(std::move(derived()), std::move(func));
}

} // namespace flux

#endif // FLUX_OP_ADJACENT_HPP_INCLUDED


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ADJACENT_FILTER_HPP_INCLUDED
#define FLUX_OP_ADJACENT_FILTER_HPP_INCLUDED



namespace flux {

namespace detail {

template <multipass_sequence Base, typename Pred>
struct adjacent_filter_adaptor
    : inline_sequence_base<adjacent_filter_adaptor<Base, Pred>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;

public:
    constexpr adjacent_filter_adaptor(decays_to<Base> auto&& base, Pred pred)
        : base_(FLUX_FWD(base)),
          pred_(std::move(pred))
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool
                = default;
        };

    public:
        using value_type = value_t<Base>;

        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type{flux::first(self.base_)};
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.base_cur);
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at(self.base_, cur.base_cur))
        {
            return flux::read_at(self.base_, cur.base_cur);
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at_unchecked(self.base_, cur.base_cur))
        {
            return flux::read_at_unchecked(self.base_, cur.base_cur);
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at(self.base_, cur.base_cur))
        {
            return flux::move_at(self.base_, cur.base_cur);
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at_unchecked(self.base_, cur.base_cur))
        {
            return flux::move_at_unchecked(self.base_, cur.base_cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            auto temp = cur.base_cur;
            flux::inc(self.base_, cur.base_cur);

            while (!flux::is_last(self.base_, cur.base_cur)) {
                if (std::invoke(self.pred_,
                                flux::read_at(self.base_, temp),
                                flux::read_at(self.base_, cur.base_cur))) {
                    break;
                }
                flux::inc(self.base_, cur.base_cur);
            }
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
            requires bidirectional_sequence<Base>
        {
            auto first = flux::first(self.base_);
            FLUX_DEBUG_ASSERT(cur.base_cur != first);

            flux::dec(self.base_, cur.base_cur);

            while (cur.base_cur != first) {
                auto temp = flux::prev(self.base_, cur.base_cur);

                if (std::invoke(self.pred_, flux::read_at(self.base_, temp),
                                flux::read_at(self.base_, cur.base_cur))) {
                    break;
                }
                cur.base_cur = std::move(temp);
            }
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            return cursor_type{flux::last(self.base_)};
        }
    };
};

struct adjacent_filter_fn {
    template <adaptable_sequence Seq, typename Pred>
        requires multipass_sequence<Seq> &&
                 std::predicate<Pred&, element_t<Seq>, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred) const -> multipass_sequence auto
    {
        return adjacent_filter_adaptor<std::decay_t<Seq>, Pred>(
            FLUX_FWD(seq), std::move(pred));
    }
};

struct dedup_fn {
    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq> &&
                 std::equality_comparable<element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> multipass_sequence auto
    {
        return adjacent_filter_fn{}(FLUX_FWD(seq), std::ranges::not_equal_to{});
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto adjacent_filter = detail::adjacent_filter_fn{};
FLUX_EXPORT inline constexpr auto dedup = detail::dedup_fn{};

template <typename D>
template <typename Pred>
    requires multipass_sequence<D> &&
             std::predicate<Pred&, element_t<D>, element_t<D>>
constexpr auto inline_sequence_base<D>::adjacent_filter(Pred pred) &&
{
    return flux::adjacent_filter(std::move(derived()), std::move(pred));
}

template <typename D>
constexpr auto inline_sequence_base<D>::dedup() &&
    requires multipass_sequence<D> &&
             std::equality_comparable<element_t<D>>
{
    return flux::dedup(std::move(derived()));
}

} // namespace flux

#endif // FLUX_OP_ADJACENT_FILTER_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ALL_ANY_NONE_HPP_INCLUDED
#define FLUX_OP_ALL_ANY_NONE_HPP_INCLUDED




namespace flux {

namespace all_detail {

struct fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr bool operator()(Seq&& seq, Pred pred) const
    {
        return is_last(seq, for_each_while(seq, [&](auto&& elem) {
            return std::invoke(pred, FLUX_FWD(elem));
        }));
    }
};

} // namespace all_detail

FLUX_EXPORT inline constexpr auto all = all_detail::fn{};

namespace none_detail {

struct fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr bool operator()(Seq&& seq, Pred pred) const
    {
        return is_last(seq, for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, FLUX_FWD(elem));
        }));
    }
};

} // namespace none_detail

FLUX_EXPORT inline constexpr auto none = none_detail::fn{};

namespace any_detail {

struct fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr bool operator()(Seq&& seq, Pred pred) const
    {
        return !is_last(seq, for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, FLUX_FWD(elem));
        }));
    }
};

} // namespace any_detail

FLUX_EXPORT inline constexpr auto any = any_detail::fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::all(Pred pred)
{
    return flux::all(derived(), std::move(pred));
}

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::any(Pred pred)
{
    return flux::any(derived(), std::move(pred));
}

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::none(Pred pred)
{
    return flux::none(derived(), std::move(pred));
}

} // namespace flux

#endif // FLUX_OP_ALL_ANY_NONE_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CACHE_LAST_HPP_INCLUDED
#define FLUX_OP_CACHE_LAST_HPP_INCLUDED




namespace flux {

namespace detail {

template <sequence Base>
struct cache_last_adaptor : inline_sequence_base<cache_last_adaptor<Base>>
{
private:
    Base base_;
    flux::optional<cursor_t<Base>> cached_last_{};

    friend struct passthrough_traits_base<Base>;

    constexpr auto base() -> Base& { return base_; }

public:
    constexpr explicit cache_last_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_sequence_traits : detail::passthrough_traits_base<Base> {

        using value_type = value_t<Base>;
        using self_t = cache_last_adaptor;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;

        static constexpr auto is_last(self_t& self, cursor_t<Base> const& cur)
        {
            if (flux::is_last(self.base_, cur)) {
                self.cached_last_ = flux::optional(cur);
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
                FLUX_DEBUG_ASSERT(self.cached_last_.has_value());
            }
            return self.cached_last_.value_unchecked();
        }
    };
};

struct cache_last_fn {
    template <adaptable_sequence Seq>
        requires bounded_sequence<Seq> ||
            (multipass_sequence<Seq> && not infinite_sequence<Seq>)
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr (bounded_sequence<Seq>) {
            return FLUX_FWD(seq);
        } else {
            return cache_last_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto cache_last = detail::cache_last_fn{};

template <typename Derived>
constexpr auto inline_sequence_base<Derived>::cache_last() &&
    requires bounded_sequence<Derived> ||
        (multipass_sequence<Derived> && not infinite_sequence<Derived>)
{
    return flux::cache_last(std::move(derived()));
}

} // namespace flux

#endif // FLUX_OP_CACHE_LAST_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CARTESIAN_BASE_HPP_INCLUDED
#define FLUX_CARTESIAN_BASE_HPP_INCLUDED

namespace flux::detail {

enum class cartesian_kind { product, power };
enum class read_kind { tuple, map };

template <typename B0, typename...>
inline constexpr bool cartesian_is_bounded = bounded_sequence<B0>;

template <typename T, std::size_t RepeatCount>
struct tuple_repeated {
    template <std::size_t I>
    using repeater = T;

    template <std::size_t... Is>
    static auto make_tuple(std::index_sequence<Is...>) -> std::tuple<repeater<Is>...>;    

    using type = decltype(make_tuple(std::make_index_sequence<RepeatCount>{}));
};

template <typename T, std::size_t RepeatCount>
using tuple_repeated_t = tuple_repeated<T, RepeatCount>::type;

template<std::size_t Arity, cartesian_kind CartesianKind, read_kind ReadKind, typename... Bases>
struct cartesian_traits_types {
};

template<std::size_t Arity, typename Base>
struct cartesian_traits_types<Arity, cartesian_kind::power, read_kind::tuple, Base> {
    using value_type = tuple_repeated_t<value_t<Base>, Arity>;
};

template<std::size_t Arity, typename... Bases>
struct cartesian_traits_types<Arity, cartesian_kind::product, read_kind::tuple, Bases...> {
    using value_type = std::tuple<value_t<Bases>...>;
};

template <std::size_t Arity, cartesian_kind CartesianKind, read_kind ReadKind, typename... Bases>
struct cartesian_traits_base_impl : default_sequence_traits {
private:

    template<std::size_t I, typename Self>
    static constexpr auto& get_base(Self& self)
        requires (CartesianKind == cartesian_kind::power)
    {
        return self.base_;
    }

    template<std::size_t I, typename Self>
    static constexpr auto& get_base(Self& self)
        requires (CartesianKind == cartesian_kind::product)
    {
        return std::get<I>(self.bases_);
    }


    template <std::size_t I, typename Self>
    static constexpr auto inc_impl(Self& self, cursor_t<Self>& cur) -> cursor_t<Self>&
    {
        flux::inc(get_base<I>(self), std::get<I>(cur));

        if constexpr (I > 0) {
            if (flux::is_last(get_base<I>(self), std::get<I>(cur))) {
                std::get<I>(cur) = flux::first(get_base<I>(self));
                inc_impl<I-1>(self, cur);
            }
        }

        return cur;
    }


    template <std::size_t I, typename Self>
    static constexpr auto dec_impl(Self& self, cursor_t<Self>& cur) -> cursor_t<Self>&
    {
        if (std::get<I>(cur) == flux::first(get_base<I>(self))) {
            std::get<I>(cur) = flux::last(get_base<I>(self));
            if constexpr (I > 0) {
                dec_impl<I-1>(self, cur);
            }
        }

        flux::dec(get_base<I>(self), std::get<I>(cur));

        return cur;
    }

    template <std::size_t I, typename Self>
    static constexpr auto ra_inc_impl(Self& self, cursor_t<Self>& cur, distance_t offset)
    -> cursor_t<Self>&
    {
        if (offset == 0) {
            return cur;
        }

        auto& base = get_base<I>(self);
        const auto this_index = flux::distance(base, flux::first(base), std::get<I>(cur));
        auto new_index = num::checked_add(this_index, offset);
        auto this_size = flux::size(base);

        // If the new index overflows the maximum or underflows zero, calculate the carryover and fix it.
        if (new_index < 0 || new_index >= this_size) {
            offset = num::checked_div(new_index, this_size);
            new_index = num::checked_mod(new_index, this_size);

            // Correct for negative index which may happen when underflowing.
            if (new_index < 0) {
                new_index = num::checked_add(new_index, this_size);
                offset = num::checked_sub(offset, flux::distance_t(1));
            }

            // Call the next level down if necessary.
            if constexpr (I > 0) {
                if (offset != 0) {
                    ra_inc_impl<I-1>(self, cur, offset);
                }
            }
        }

        flux::inc(base, std::get<I>(cur), num::checked_sub(new_index, this_index));

        return cur;
    }

    template <std::size_t I, typename Self>
    static constexpr auto distance_impl(Self& self,
                                        cursor_t<Self> const& from,
                                        cursor_t<Self> const& to) -> distance_t
    {
        if constexpr (I == 0) {
            return flux::distance(get_base<0>(self), std::get<0>(from), std::get<0>(to));
        } else {
            auto prev_dist = distance_impl<I-1>(self, from, to);
            auto our_sz = flux::size(get_base<I>(self));
            auto our_dist = flux::distance(get_base<I>(self), std::get<I>(from), std::get<I>(to));
            return prev_dist * our_sz + our_dist;
        }
    }

    template <std::size_t I, typename Self, typename Fn>
    static constexpr auto read1_(Fn fn, Self& self, cursor_t<Self> const& cur)
    -> decltype(auto)
    {
        return fn(get_base<I>(self), std::get<I>(cur));
    }

    template <typename Fn, typename Self>
    static constexpr auto read_(Fn& fn, Self& self, cursor_t<Self> const& cur)
    -> decltype(auto)
        requires (ReadKind == read_kind::tuple)
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) {
            return std::tuple<decltype(read1_<N>(fn, self, cur))...>(read1_<N>(fn, self, cur)...);
        }(std::make_index_sequence<Arity>{});
    }

    template <std::size_t I, typename Self, typename Function,
            typename... PartialElements>
    static constexpr void for_each_while_impl(Self& self,
                                              bool& keep_going,
                                              cursor_t<Self>& cur,
                                              Function&& func,
                                              PartialElements&&... partial_elements)
    {
        // We need to iterate right to left.
        if constexpr (I == Arity - 1) {
            std::get<I>(cur) = flux::for_each_while(get_base<I>(self),
                [&](auto&& elem) {
                    keep_going = std::invoke(func,
                                             element_t<Self>(FLUX_FWD(partial_elements)..., FLUX_FWD(elem)));
                    return keep_going;
                });
        } else {
            std::get<I>(cur) = flux::for_each_while(get_base<I>(self),
                [&](auto&& elem) {
                    for_each_while_impl<I+1>(
                            self, keep_going, cur,
                            func, FLUX_FWD(partial_elements)..., FLUX_FWD(elem));
                    return keep_going;
                });
        }
    }

protected:
    using types = cartesian_traits_types<Arity, CartesianKind, ReadKind, Bases...>;

public:

    template <typename Self>
    static constexpr auto first(Self& self)
        requires (CartesianKind == cartesian_kind::product)
    {
        return std::apply([](auto&&... args) {
            return std::tuple(flux::first(FLUX_FWD(args))...);
        }, self.bases_);
    }

    template <typename Self>
    static constexpr auto first(Self& self)
        requires (CartesianKind == cartesian_kind::power)
    {
        auto base_cur = flux::first(self.base_);
        return [&base_cur]<std::size_t... Is>(std::index_sequence<Is...>) {
            static_assert(sizeof...(Bases) == 1);
            std::array<cursor_t<Bases>..., Arity> cur = {(static_cast<void>(Is), base_cur)...};
            return cur;
        }(std::make_index_sequence<Arity>{});
    }

    template <typename Self>
    static constexpr auto size(Self& self) -> distance_t
        requires (CartesianKind == cartesian_kind::product
                  && (sized_sequence<Bases> && ...))
    {
        return std::apply([](auto& base0, auto&... bases) {
            distance_t sz = flux::size(base0);
            ((sz = num::checked_mul(sz, flux::size(bases))), ...);
            return sz;
        }, self.bases_);
    }

    template <typename Self>
    static constexpr auto size(Self& self) -> distance_t
        requires (CartesianKind == cartesian_kind::power
                  && (sized_sequence<Bases> && ...))
    {
        return num::checked_pow(flux::size(self.base_), Arity);
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, cursor_t<Self> const& cur) -> bool
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) {
            return (flux::is_last(get_base<N>(self), std::get<N>(cur)) || ...);
        }(std::make_index_sequence<Arity>{});
    }

    template <typename Self>
    static constexpr auto dec(Self& self, cursor_t<Self>& cur) -> cursor_t<Self>&
        requires ((bidirectional_sequence<Bases> && ...) &&
                  (bounded_sequence<Bases> && ...))
    {
        return dec_impl<Arity - 1>(self, cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur, distance_t offset) -> cursor_t<Self>&
        requires ((random_access_sequence<Bases> && ...) &&
                  (sized_sequence<Bases> && ...))
    {
        return ra_inc_impl<Arity - 1>(self, cur, offset);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur) -> cursor_t<Self>&
    {
        return inc_impl<Arity - 1>(self, cur);
    }

    template <typename Self>
    static constexpr auto last(Self& self) -> cursor_t<Self>
        requires cartesian_is_bounded<Bases...>
    {
        if constexpr (CartesianKind == cartesian_kind::product) {
            auto cur = first(self);
            bool any_is_empty = std::apply([](auto& /*ignored*/, auto&... bases) {
                    return (flux::is_empty(bases) || ...);
                }, self.bases_);
            if (!any_is_empty) {
                std::get<0>(cur) = flux::last(get_base<0>(self));
            }
            return cur;
        } else {
            auto cur = first(self);
            std::get<0>(cur) = flux::last(get_base<0>(self));
            return cur;
        }
    }

    template <typename Self>
    static constexpr auto distance(Self& self,
                                        cursor_t<Self> const& from,
                                        cursor_t<Self> const& to) -> distance_t
        requires ((random_access_sequence<Bases> && ...) &&
                  (sized_sequence<Bases> && ...))
    {
        return distance_impl<Arity - 1>(self, from, to);
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
        requires (ReadKind == read_kind::tuple)
    {
        return read_(flux::read_at, self, cur);
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
        requires (ReadKind == read_kind::map)
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) -> decltype(auto) {
            return std::invoke(self.func_, flux::read_at(get_base<N>(self), std::get<N>(cur))...);
        }(std::make_index_sequence<Arity>{});
    }

    template <typename Self>
    static constexpr auto read_at_unchecked(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
        requires (ReadKind == read_kind::map)
    {
        return [&]<std::size_t... N>(std::index_sequence<N...>) -> decltype(auto) {
            return std::invoke(self.func_, flux::read_at_unchecked(get_base<N>(self), std::get<N>(cur))...);
        }(std::make_index_sequence<Arity>{});
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
        requires (ReadKind == read_kind::map)
    {
        return default_sequence_traits::move_at(self, cur);
    }

    template <typename Self>
    static constexpr auto move_at_unchecked(Self& self, cursor_t<Self> const& cur)
        -> decltype(auto)
        requires (ReadKind == read_kind::map)
    {
        return default_sequence_traits::move_at_unchecked(self, cur);
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_t<Self> const& cur)
        requires (ReadKind == read_kind::tuple)
    {
        return read_(flux::move_at, self, cur);
    }

    template <typename Self>
    static constexpr auto read_at_unchecked(Self& self, cursor_t<Self> const& cur)
        requires (ReadKind == read_kind::tuple)
    {
        return read_(flux::read_at_unchecked, self, cur);
    }

    template <typename Self>
    static constexpr auto move_at_unchecked(Self& self, cursor_t<Self> const& cur)
        requires (ReadKind == read_kind::tuple)
    {
        return read_(flux::move_at_unchecked, self, cur);
    }

    template <typename Self, typename Function>
    static constexpr auto for_each_while(Self& self, Function&& func)
    -> cursor_t<Self>
        requires (ReadKind == read_kind::tuple)
    {
        bool keep_going = true;
        cursor_t<Self> cur;
        for_each_while_impl<0>(self, keep_going, cur, FLUX_FWD(func));
        return cur;
    }

    template <typename Self, typename Function>
    static constexpr auto for_each_while(Self& self, Function&& func) -> cursor_t<Self>
        requires (ReadKind == read_kind::map)
    {
        return default_sequence_traits::for_each_while(self, FLUX_FWD(func));
    }

};

template <std::size_t Arity, cartesian_kind CartesianKind, read_kind ReadKind, typename... Bases>
struct cartesian_traits_base : cartesian_traits_base_impl<Arity, CartesianKind, ReadKind, Bases...> {
    using impl = cartesian_traits_base_impl<Arity, CartesianKind, ReadKind, Bases...>;
};

template <std::size_t Arity, cartesian_kind CartesianKind, typename... Bases>
struct cartesian_traits_base<Arity, CartesianKind, read_kind::tuple, Bases...> : cartesian_traits_base_impl<Arity, CartesianKind, read_kind::tuple, Bases...> {
    using impl = cartesian_traits_base_impl<Arity, CartesianKind, read_kind::tuple, Bases...>;

    using value_type = typename impl::types::value_type;
};

}

#endif //FLUX_CARTESIAN_BASE_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_POWER_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_POWER_HPP_INCLUDED






#include <tuple>

namespace flux {

namespace detail {

template <std::size_t PowN, sequence Base>
struct cartesian_power_adaptor
    : inline_sequence_base<cartesian_power_adaptor<PowN, Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;

public:
    constexpr explicit cartesian_power_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    using flux_sequence_traits = cartesian_traits_base<
        PowN,
        cartesian_kind::power,
        read_kind::tuple,
        Base
    >;
    friend flux_sequence_traits::impl;
};



template<std::size_t PowN>
struct cartesian_power_fn {

    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr(PowN == 0) {
            return empty<std::tuple<>>;
        } else {
            return cartesian_power_adaptor<PowN, std::decay_t<Seq>>(
                FLUX_FWD(seq));
        }
    }
};


} // end namespace detail

FLUX_EXPORT
template<distance_t N>
    requires (N >= 0)
inline constexpr auto cartesian_power = detail::cartesian_power_fn<N>{};

} // end namespace flux

#endif



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_POWER_MAP_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_POWER_MAP_HPP_INCLUDED




namespace flux {

namespace detail {

template <sequence Base, std::size_t PowN, typename Func>
struct cartesian_power_map_adaptor
    : inline_sequence_base<cartesian_power_map_adaptor<Base, PowN, Func>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

public:
    constexpr explicit cartesian_power_map_adaptor(decays_to<Base> auto&& base, decays_to<Func> auto&& func)
        : base_(FLUX_FWD(base)),
          func_(FLUX_FWD(func))
    {}

    using flux_sequence_traits = cartesian_traits_base<
        PowN,
        cartesian_kind::power,
        read_kind::map,
        Base
    >;
    friend flux_sequence_traits::impl;
};

template <std::size_t PowN>
struct cartesian_power_map_fn
{
    template <adaptable_sequence Seq, typename Func>
        requires multipass_sequence<Seq> &&
        detail::repeated_invocable<Func&, element_t<Seq>, PowN>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func) const
    {
        if constexpr(PowN == 0) {
            return empty<std::invoke_result_t<Func>>;
        } else {
            return cartesian_power_map_adaptor<std::decay_t<Seq>, PowN, Func>(
                FLUX_FWD(seq), std::move(func));
        }
    }
};

} // namespace detail

FLUX_EXPORT
template <distance_t N>
    requires (N >= 0)
inline constexpr auto cartesian_power_map = detail::cartesian_power_map_fn<N>{};

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_PRODUCT_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_PRODUCT_HPP_INCLUDED






#include <tuple>

namespace flux {

namespace detail {

template <sequence... Bases>
struct cartesian_product_adaptor
    : inline_sequence_base<cartesian_product_adaptor<Bases...>> {
private:
    FLUX_NO_UNIQUE_ADDRESS std::tuple<Bases...> bases_;

public:
    constexpr explicit cartesian_product_adaptor(decays_to<Bases> auto&&... bases)
        : bases_(FLUX_FWD(bases)...)
    {}
    
    using flux_sequence_traits = cartesian_traits_base<
        sizeof...(Bases),
        cartesian_kind::product,
        read_kind::tuple,
        Bases...
    >;
    friend flux_sequence_traits::impl;
};

struct cartesian_product_fn {
    template <adaptable_sequence Seq0, adaptable_sequence... Seqs>
        requires (multipass_sequence<Seqs> && ...)
    [[nodiscard]]
    constexpr auto operator()(Seq0&& seq0, Seqs&&... seqs) const
    {
        return cartesian_product_adaptor<std::decay_t<Seq0>, std::decay_t<Seqs>...>(
                    FLUX_FWD(seq0), FLUX_FWD(seqs)...);
    }
};

} // end namespace detail

FLUX_EXPORT inline constexpr auto cartesian_product = detail::cartesian_product_fn{};


} // end namespace flux

#endif



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CARTESIAN_PRODUCT_MAP_HPP_INCLUDED
#define FLUX_OP_CARTESIAN_PRODUCT_MAP_HPP_INCLUDED



namespace flux {

namespace detail {

template <typename Func, sequence... Bases>
struct cartesian_product_map_adaptor
    : inline_sequence_base<cartesian_product_map_adaptor<Func, Bases...>> {
private:
    FLUX_NO_UNIQUE_ADDRESS std::tuple<Bases...> bases_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

public:
    constexpr explicit cartesian_product_map_adaptor(decays_to<Func> auto&& func, decays_to<Bases> auto&&... bases)
        : bases_(FLUX_FWD(bases)...),
          func_(FLUX_FWD(func))
    {}

    using flux_sequence_traits = cartesian_traits_base<
        sizeof...(Bases),
        cartesian_kind::product,
        read_kind::map,
        Bases...
    >;
    friend flux_sequence_traits::impl;
};

struct cartesian_product_map_fn
{
    template <typename Func, adaptable_sequence Seq0, adaptable_sequence... Seqs>
        requires (multipass_sequence<Seqs> && ...) &&
        std::regular_invocable<Func&, element_t<Seq0>, element_t<Seqs>...>
    [[nodiscard]]
    constexpr auto operator()(Func func, Seq0&& seq0, Seqs&&... seqs) const
    {
        return cartesian_product_map_adaptor<Func, std::decay_t<Seq0>, std::decay_t<Seqs>...>(
                    std::move(func), FLUX_FWD(seq0), FLUX_FWD(seqs)...);
    }
};



} // namespace detail

FLUX_EXPORT inline constexpr auto cartesian_product_map = detail::cartesian_product_map_fn{};

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CHAIN_HPP_INCLUDED
#define FLUX_OP_CHAIN_HPP_INCLUDED



#include <tuple>
#include <variant>

namespace flux {

namespace detail {

template <sequence... Bases>
struct chain_adaptor : inline_sequence_base<chain_adaptor<Bases...>> {
private:
    std::tuple<Bases...> bases_;

    friend struct sequence_traits<chain_adaptor>;

public:
    explicit constexpr chain_adaptor(decays_to<Bases> auto&&... bases)
        : bases_(FLUX_FWD(bases)...)
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
    [[nodiscard]]
    constexpr auto operator()(Seqs&&... seqs) const
    {
        if constexpr (sizeof...(Seqs) == 1) {
            return std::forward<Seqs...>(seqs...);
        } else {
            return chain_adaptor<std::decay_t<Seqs>...>(FLUX_FWD(seqs)...);
        }
    }
};

} // namespace detail

template <typename... Bases>
struct sequence_traits<detail::chain_adaptor<Bases...>> : default_sequence_traits {

    using value_type = std::common_type_t<value_t<Bases>...>;

    static constexpr bool disable_multipass = !(multipass_sequence<Bases> && ...);
    static constexpr bool is_infinite = (infinite_sequence<Bases> || ...);

private:
    static constexpr std::size_t End = sizeof...(Bases) - 1;

    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    using cursor_type = std::variant<cursor_t<Bases>...>;

    template <std::size_t N, typename Self>
    static constexpr auto first_impl(Self& self) -> cursor_type
    {
        auto& base = std::get<N>(self.bases_);
        auto cur = flux::first(base);

        if constexpr (N < End) {
            if (!flux::is_last(base, cur)) {
                return cursor_type(std::in_place_index<N>, std::move(cur));
            } else {
                return first_impl<N+1>(self);
            }
        } else {
            return cursor_type(std::in_place_index<N>, std::move(cur));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto inc_impl(Self& self, cursor_type& cur) -> void
    {
        if constexpr (N < End) {
            if (cur.index() == N) {
                auto& base = std::get<N>(self.bases_);
                auto& base_cur = std::get<N>(cur);
                flux::inc(base, base_cur);
                if (flux::is_last(base, base_cur)) {
                    cur = first_impl<N + 1>(self);
                }
            } else {
                inc_impl<N+1>(self, cur);
            }
        } else {
            flux::inc(std::get<N>(self.bases_), std::get<N>(cur));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto dec_impl(Self& self, cursor_type& cur)
    {
        if constexpr (N > 0) {
            if (cur.index() == N) {
                auto& base = std::get<N>(self.bases_);
                auto& base_cur = std::get<N>(cur);

                if (base_cur == flux::first(base)) {
                    cur = cursor_type(std::in_place_index<N-1>,
                                      flux::last(std::get<N-1>(self.bases_)));
                    dec_impl<N-1>(self, cur);
                } else {
                    flux::dec(base, base_cur);
                }
            } else {
                dec_impl<N-1>(self, cur);
            }
        } else {
            flux::dec(std::get<0>(self.bases_), std::get<0>(cur));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto read_impl(Self& self, cursor_type const& cur)
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
    static constexpr auto move_impl(Self& self, cursor_type const& cur)
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
        -> cursor_type
    {
        if constexpr (N < End) {
            auto& base = std::get<N>(self.bases_);
            auto base_cur = flux::for_each_while(base, pred);
            if (!flux::is_last(base, base_cur)) {
                return cursor_type(std::in_place_index<N>, std::move(base_cur));
            } else {
                return for_each_while_impl<N+1>(self, pred);
            }
        } else {
            return cursor_type(std::in_place_index<N>,
                               flux::for_each_while(std::get<N>(self.bases_), pred));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto distance_impl(Self& self,
                                        cursor_type const& from,
                                        cursor_type const& to)
    {
        if constexpr (N < End) {
            if (N < from.index()) {
                return distance_impl<N+1>(self, from, to);
            }

            FLUX_DEBUG_ASSERT(N == from.index());
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
            FLUX_DEBUG_ASSERT(N == from.index() && N == to.index());
            return flux::distance(std::get<N>(self.bases_), std::get<N>(from), std::get<N>(to));
        }
    }

    template <std::size_t N, typename Self>
    static constexpr auto inc_ra_impl(Self& self, cursor_type& cur,
                                      distance_t offset)
        -> cursor_type&
    {
        if constexpr (N < End) {
            if (N < cur.index()) {
                return inc_ra_impl<N+1>(self, cur, offset);
            }

            FLUX_DEBUG_ASSERT(cur.index() == N);
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
            FLUX_DEBUG_ASSERT(cur.index() == N);
            flux::inc(std::get<N>(self.bases_), std::get<N>(cur), offset);
            return cur;
        }
    }

public:
    template <typename Self>
    static constexpr auto first(Self& self) -> cursor_type
    {
        return first_impl<0>(self);
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
    {
        return cur.index() == End &&
               flux::is_last(std::get<End>(self.bases_), std::get<End>(cur));
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_type const& cur)
        -> std::common_reference_t<element_t<const_like_t<Self, Bases>>...>
    {
        return read_impl<0>(self, cur);
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, cursor_type const& cur)
        -> std::common_reference_t<rvalue_element_t<const_like_t<Self, Bases>>...>
    {
        return move_impl<0>(self, cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_type& cur) -> void
    {
        inc_impl<0>(self, cur);
    }

    template <typename Self>
    static constexpr auto dec(Self& self, cursor_type& cur) -> void
        requires (bidirectional_sequence<const_like_t<Self, Bases>> && ...) &&
                 (bounded_sequence<const_like_t<Self,Bases>> &&...)
    {
        dec_impl<End>(self, cur);
    }

    template <typename Self>
    static constexpr auto last(Self& self) -> cursor_type
        requires bounded_sequence<decltype(std::get<End>(self.bases_))>
    {
        return cursor_type(std::in_place_index<End>, flux::last(std::get<End>(self.bases_)));
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
    static constexpr auto distance(Self& self, cursor_type const& from,
                                   cursor_type const& to)
        -> distance_t
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
    static constexpr auto inc(Self& self, cursor_type& cur, distance_t offset)
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...) &&
                 (bounded_sequence<const_like_t<Self, Bases>> && ...)
    {
        inc_ra_impl<0>(self, cur, offset);
    }

};

FLUX_EXPORT inline constexpr auto chain = detail::chain_fn{};

} // namespace flux

#endif


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CHUNK_HPP_INCLUDED
#define FLUX_OP_CHUNK_HPP_INCLUDED




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

template <sequence Base, bool Bounded>
    requires (!Bounded || regular_cursor<cursor_t<Base>>)
struct subsequence : inline_sequence_base<subsequence<Base, Bounded>>
{
private:
    Base* base_;
    FLUX_NO_UNIQUE_ADDRESS slice_data<cursor_t<Base>, Bounded> data_;

    friend struct sequence_traits<subsequence>;

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
        { traits_t<Seq>::slice(seq, std::move(cur)) } -> sequence;
        { traits_t<Seq>::slice(seq, std::move(cur), std::move(cur)) } -> sequence;
    };

struct slice_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq> from,
                              cursor_t<Seq> to) const
        -> sequence auto
    {
        if constexpr (has_overloaded_slice<Seq>) {
            return traits_t<Seq>::slice(seq, std::move(from), std::move(to));
        } else {
            return subsequence(seq, std::move(from), std::move(to));
        }
    }

    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq> from, last_fn) const
        -> sequence auto
    {
        if constexpr (has_overloaded_slice<Seq>) {
            return traits_t<Seq>::slice(seq, std::move(from));
        } else {
            return subsequence(seq, std::move(from));
        }
    }
};

} // namespace detail

using detail::subsequence;

template <typename Base, bool Bounded>
struct sequence_traits<subsequence<Base, Bounded>>
    : detail::passthrough_traits_base<Base>
{
    using value_type = value_t<Base>;
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

    using default_sequence_traits::size;
    using default_sequence_traits::for_each_while;
};

FLUX_EXPORT inline constexpr auto slice = detail::slice_fn{};

#if 0
template <typename Derived>
template <std::same_as<Derived> D>
constexpr auto inline_sequence_base<Derived>::slice(cursor_t<D> from, cursor_t<D> to) &
{
    return flux::slice(derived(), std::move(from), std::move(to));
}

template <typename Derived>
template <std::same_as<Derived> D>
constexpr auto inline_sequence_base<Derived>::slice_from(cursor_t<D> from) &
{
    return flux::slice(derived(), std::move(from));
}
#endif

} // namespace flux

#endif // namespace FLUX_OP_SLICE_HPP_INCLUDED


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_STRIDE_HPP_INCLUDED
#define FLUX_OP_STRIDE_HPP_INCLUDED



namespace flux {

namespace detail {

// This is a Flux-ified version of ranges::advance.
inline constexpr struct advance_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur, distance_t offset) const -> distance_t
    {
        if (offset > 0) {
            distance_t counter = 0;
            while (offset-- > 0 && !flux::is_last(seq, cur))  {
                flux::inc(seq, cur);
                ++counter;
            }
            return counter;
        } else if (offset < 0) {
            if constexpr (bidirectional_sequence<Seq>) {
                auto const fst = flux::first(seq);
                while (offset < 0) {
                    if (flux::dec(seq, cur) == fst) {
                        break;
                    }
                    ++offset;
                }
                return offset;
            } else {
                runtime_error("advance() called with negative offset and non-bidirectional sequence");
            }
        } else {
            return 0;
        }
    }

    template <random_access_sequence Seq>
        requires bounded_sequence<Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur, distance_t offset) const -> distance_t
    {
        if (offset > 0) {
            auto dist = std::min(flux::distance(seq, cur, flux::last(seq)), offset);
            flux::inc(seq, cur, dist);
            return offset - dist;
        } else if (offset < 0) {
            auto dist = -std::min(flux::distance(seq, flux::first(seq), cur), -offset);
            flux::inc(seq, cur, dist);
            return offset - dist;
        } else {
            return 0;
        }
    }
} advance;

template <typename Base>
struct stride_adaptor : inline_sequence_base<stride_adaptor<Base>> {
private:
    Base base_;
    distance_t stride_;

public:
    constexpr stride_adaptor(decays_to<Base> auto&& base, distance_t stride)
        : base_(FLUX_FWD(base)),
          stride_(stride)
    {}

    constexpr auto base() & -> Base& { return base_; }
    constexpr auto base() const& -> Base const& { return base_; }

    struct flux_sequence_traits : passthrough_traits_base<Base> {

        using value_type = value_t<Base>;
        static inline constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto inc(auto& self, cursor_t<Base>& cur) -> void
        {
            advance(self.base(), cur, self.stride_);
        }

        // This version of stride is never bidir
        static void dec(...) = delete;

        static constexpr auto size(auto& self) -> distance_t
            requires sized_sequence<Base>
        {
            auto s = flux::size(self.base_);
            return s/self.stride_ + (s % self.stride_ == 0 ? 0 : 1);
        }

        static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_t<Base>
            requires sequence<decltype((self.base_))>
        {
            distance_t n = self.stride_;
            return flux::for_each_while(self.base_, [&n, &pred, s = self.stride_](auto&& elem) {
                if (++n < s) {
                    return true;
                } else {
                    n = 0;
                    return std::invoke(pred, FLUX_FWD(elem));
                }
            });
        }

    };
};

template <bidirectional_sequence Base>
struct stride_adaptor<Base> : inline_sequence_base<stride_adaptor<Base>> {
private:
    Base base_;
    distance_t stride_;

public:
    constexpr stride_adaptor(decays_to<Base> auto&& base, distance_t stride)
        : base_(FLUX_FWD(base)),
          stride_(stride)
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> cur{};
            distance_t missing = 0;

            friend constexpr auto operator==(cursor_type const& lhs, cursor_type const& rhs) -> bool
            {
                return lhs.cur == rhs.cur;
            }

            friend constexpr auto operator<=>(cursor_type const& lhs, cursor_type const& rhs)
                -> std::strong_ordering
                requires ordered_cursor<cursor_t<Base>>
            {
                return lhs.cur <=> rhs.cur;
            }
        };

    public:
        using value_type = value_t<Base>;
        static constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type {
                .cur = flux::first(self.base_),
                .missing = 0
            };
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            cur.missing = advance(self.base_, cur.cur, self.stride_);
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at(self.base_, cur.cur))
        {
            return flux::read_at(self.base_, cur.cur);
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at(self.base_, cur.cur))
        {
            return flux::move_at(self.base_, cur.cur);
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at_unchecked(self.base_, cur.cur))
        {
            return flux::read_at_unchecked(self.base_, cur.cur);
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at_unchecked(self.base_, cur.cur))
        {
            return flux::move_at_unchecked(self.base_, cur.cur);
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<Base> && sized_sequence<Base>
        {
            distance_t missing =
                (self.stride_ - flux::size(self.base_) % self.stride_) % self.stride_;
            return cursor_type{
                .cur = flux::last(self.base_),
                .missing = missing
            };
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
            requires bidirectional_sequence<Base>
        {
            advance(self.base_, cur.cur, cur.missing - self.stride_);
            cur.missing = 0;
        }

        static constexpr auto size(auto& self) -> distance_t
            requires sized_sequence<Base>
        {
            auto s = flux::size(self.base_);
            return s/self.stride_ + (s % self.stride_ == 0 ? 0 : 1);
        }

        static constexpr auto distance(auto& self, cursor_type const& from,
                                       cursor_type const& to) -> distance_t
            requires random_access_sequence<Base>
        {
            return (flux::distance(self.base_, from.cur, to.cur) - from.missing + to.missing)/self.stride_;
        }

        static constexpr auto inc(auto& self, cursor_type& cur, distance_t offset) -> void
            requires random_access_sequence<Base>
        {
            if (offset > 0) {
                cur.missing = advance(self.base_, cur.cur, num::checked_mul(offset, self.stride_)) % self.stride_;
            } else if (offset < 0) {
                advance(self.base_, cur.cur, num::checked_add(num::checked_mul(offset, self.stride_), cur.missing));
                cur.missing = 0;
            }
        }

        static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_type
            requires sequence<decltype((self.base_))>
        {
            distance_t n = self.stride_;
            auto c = flux::for_each_while(self.base_, [&n, &pred, s = self.stride_](auto&& elem) {
                if (++n < s) {
                    return true;
                } else {
                    n = 0;
                    return std::invoke(pred, FLUX_FWD(elem));
                }
            });
            return cursor_type{std::move(c), (n + 1) % self.stride_};
        }
    };
};

struct stride_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, std::integral auto by) const
    {
        FLUX_ASSERT(by > 0);
        return stride_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq), checked_cast<distance_t>(by));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto stride = detail::stride_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::stride(std::integral auto by) &&
{
    return flux::stride(std::move(derived()), by);
}

} // namespace flux

#endif // FLUX_OP_STRIDE_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_TAKE_HPP_INCLUDED
#define FLUX_OP_TAKE_HPP_INCLUDED





namespace flux {

namespace detail {

template <sequence Base>
struct take_adaptor : inline_sequence_base<take_adaptor<Base>>
{
private:
    Base base_;
    distance_t count_;

public:
    constexpr take_adaptor(decays_to<Base> auto&& base, distance_t count)
        : base_(FLUX_FWD(base)),
          count_(count)
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base&& { return std::move(base_); }

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;
            distance_t length;

            friend bool operator==(cursor_type const&, cursor_type const&) = default;
            friend auto operator<=>(cursor_type const& lhs, cursor_type const& rhs) = default;
        };

    public:
        using value_type = value_t<Base>;

        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type{.base_cur = flux::first(self.base_),
                               .length = self.count_};
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return cur.length <= 0 || flux::is_last(self.base_, cur.base_cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur)
        {
            flux::inc(self.base_, cur.base_cur);
            cur.length = num::checked_sub(cur.length, distance_t{1});
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at(self.base_, cur.base_cur))
        {
            return flux::read_at(self.base_, cur.base_cur);
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at(self.base_, cur.base_cur))
        {
            return flux::move_at(self.base_, cur.base_cur);
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at_unchecked(self.base_, cur.base_cur))
        {
            return flux::read_at_unchecked(self.base_, cur.base_cur);
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at_unchecked(self.base_, cur.base_cur))
        {
            return flux::move_at_unchecked(self.base_, cur.base_cur);
        }

        static constexpr auto dec(auto& self, cursor_type& cur)
            requires bidirectional_sequence<Base>
        {
            flux::dec(self.base_, cur.base_cur);
            cur.length = num::checked_add(cur.length, distance_t{1});
        }

        static constexpr auto inc(auto& self, cursor_type& cur, distance_t offset)
            requires random_access_sequence<Base>
        {
            flux::inc(self.base_, cur.base_cur, offset);
            cur.length = num::checked_sub(cur.length, offset);
        }

        static constexpr auto distance(auto& self, cursor_type const& from, cursor_type const& to)
            -> distance_t
            requires random_access_sequence<Base>
        {
            return (std::min)(flux::distance(self.base_, from.base_cur, to.base_cur),
                              num::checked_sub(from.length, to.length));
        }

        static constexpr auto data(auto& self)
            -> decltype(flux::data(self.base_))
            requires contiguous_sequence<Base>
        {
            return flux::data(self.base_);
        }

        static constexpr auto size(auto& self)
            requires sized_sequence<Base> || infinite_sequence<Base>
        {
            if constexpr (infinite_sequence<Base>) {
                return self.count_;
            } else {
                return (std::min)(flux::size(self.base_), self.count_);
            }
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires (random_access_sequence<Base> && sized_sequence<Base>) ||
                      infinite_sequence<Base>
        {
            return cursor_type{
                .base_cur = flux::next(self.base_, flux::first(self.base_), size(self)),
                .length = 0
            };
        }

        static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_type
        {
            distance_t len = self.count_;
            auto cur = flux::for_each_while(self.base_, [&](auto&& elem) {
                return (len-- > 0) && std::invoke(pred, FLUX_FWD(elem));
            });

            return cursor_type{.base_cur = std::move(cur), .length = ++len};
        }
    };
};

struct take_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, std::integral auto count) const
    {
        auto count_ = checked_cast<distance_t>(count);
        if (count_ < 0) {
            runtime_error("Negative argument passed to take()");
        }

        return take_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq), count_);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto take = detail::take_fn{};

template <typename Derived>
constexpr auto inline_sequence_base<Derived>::take(std::integral auto count) &&
{
    return flux::take(std::move(derived()), count);
}

} // namespace flux

#endif // FLUX_OP_TAKE_HPP_INCLUDED


namespace flux {

namespace detail {

template <typename Base>
struct chunk_adaptor : inline_sequence_base<chunk_adaptor<Base>> {
private:
    Base base_;
    distance_t chunk_sz_;
    optional<cursor_t<Base>> cur_ = nullopt;
    distance_t rem_ = chunk_sz_;

public:
    constexpr chunk_adaptor(decays_to<Base> auto&& base, distance_t chunk_sz)
        : base_(FLUX_FWD(base)),
          chunk_sz_(chunk_sz)
    {}

    chunk_adaptor(chunk_adaptor&&) = default;
    chunk_adaptor& operator=(chunk_adaptor&&) = default;

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct outer_cursor {
            outer_cursor(outer_cursor&&) = default;
            outer_cursor& operator=(outer_cursor&&) = default;

            friend struct flux_sequence_traits;

        private:
            explicit outer_cursor() = default;
        };

        using self_t = chunk_adaptor;

    public:
        struct value_type : inline_sequence_base<value_type> {
        private:
            chunk_adaptor* parent_;
            constexpr explicit value_type(chunk_adaptor& parent)
                : parent_(std::addressof(parent))
            {}

            friend struct self_t::flux_sequence_traits;

        public:
            value_type(value_type&&) = default;
            value_type& operator=(value_type&&) = default;

            struct flux_sequence_traits : default_sequence_traits {
            private:
                struct inner_cursor {
                    inner_cursor(inner_cursor&&) = default;
                    inner_cursor& operator=(inner_cursor&&) = default;

                    friend struct value_type::flux_sequence_traits;

                private:
                    explicit inner_cursor() = default;
                };

            public:
                static constexpr auto first(value_type&) -> inner_cursor
                {
                    return inner_cursor{};
                }

                static constexpr auto is_last(value_type& self, inner_cursor const&) -> bool
                {
                    return self.parent_->rem_ == 0;
                }

                static constexpr auto inc(value_type& self, inner_cursor&) -> void
                {
                    flux::inc(self.parent_->base_, *self.parent_->cur_);
                    if (flux::is_last(self.parent_->base_, *self.parent_->cur_)) {
                        self.parent_->rem_ = 0;
                    } else {
                        --self.parent_->rem_;
                    }
                }

                static constexpr auto read_at(value_type& self, inner_cursor const&)
                    -> element_t<Base>
                {
                    return flux::read_at(self.parent_->base_, *self.parent_->cur_);
                }
            };
        };

        static constexpr auto first(self_t& self) -> outer_cursor
        {
            self.cur_ = optional<cursor_t<Base>>(flux::first(self.base_));
            self.rem_ = self.chunk_sz_;
            return outer_cursor{};
        }

        static constexpr auto is_last(self_t& self, outer_cursor const&) -> bool
        {
            return self.rem_ != 0 && flux::is_last(self.base_, *self.cur_);
        }

        static constexpr auto inc(self_t& self, outer_cursor&) -> void
        {
            advance(self.base_, *self.cur_, self.rem_);
            self.rem_ = self.chunk_sz_;
        }

        static constexpr auto read_at(self_t& self, outer_cursor const&) -> value_type
        {
            return value_type(self);
        }

        static constexpr auto size(self_t& self) -> distance_t
            requires sized_sequence<Base>
        {
            auto s = flux::size(self.base_);
            return s/self.chunk_sz_ + (s % self.chunk_sz_ == 0 ? 0 : 1);
        }
    };
};

template <multipass_sequence Base>
struct chunk_adaptor<Base> : inline_sequence_base<chunk_adaptor<Base>> {
private:
    Base base_;
    distance_t chunk_sz_;

public:
    constexpr chunk_adaptor(decays_to<Base> auto&& base, distance_t chunk_sz)
        : base_(FLUX_FWD(base)),
          chunk_sz_(chunk_sz)
    {}

    struct flux_sequence_traits : default_sequence_traits {
        static inline constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto first(auto& self) -> cursor_t<Base>
        {
            return flux::first(self.base_);
        }

        static constexpr auto is_last(auto& self, cursor_t<Base> const& cur) -> bool
        {
            return flux::is_last(self.base_, cur);
        }

        static constexpr auto inc(auto& self, cursor_t<Base>& cur) -> void
        {
            advance(self.base_, cur, self.chunk_sz_);
        }

        static constexpr auto read_at(auto& self, cursor_t<Base> const& cur)
            -> decltype(flux::take(flux::slice(self.base_, cur, flux::last), self.chunk_sz_))
            requires multipass_sequence<decltype((self.base_))>
        {
            return flux::take(flux::slice(self.base_, cur, flux::last), self.chunk_sz_);
        }

        static constexpr auto last(auto& self) -> cursor_t<Base>
            requires bounded_sequence<Base>
        {
            return flux::last(self.base_);
        }

        static constexpr auto size(auto& self) -> distance_t
            requires sized_sequence<Base>
        {
            auto s = flux::size(self.base_);
            return s/self.chunk_sz_ + (s % self.chunk_sz_ == 0 ? 0 : 1);
        }
    };
};

template <bidirectional_sequence Base>
struct chunk_adaptor<Base> : inline_sequence_base<chunk_adaptor<Base>> {
private:
    Base base_;
    distance_t chunk_sz_;

public:
    constexpr chunk_adaptor(decays_to<Base> auto&& base, distance_t chunk_sz)
        : base_(FLUX_FWD(base)),
          chunk_sz_(chunk_sz)
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> cur{};
            distance_t missing = 0;

            friend constexpr auto operator==(cursor_type const& lhs, cursor_type const& rhs) -> bool
            {
                return lhs.cur == rhs.cur;
            }

            friend constexpr auto operator<=>(cursor_type const& lhs, cursor_type const& rhs)
                -> std::strong_ordering
                requires ordered_cursor<cursor_t<Base>>
            {
                return lhs.cur <=> rhs.cur;
            }
        };

    public:

        static inline constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type{
                .cur = flux::first(self.base_),
                .missing = 0
            };
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            cur.missing = advance(self.base_, cur.cur, self.chunk_sz_);
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            requires sequence<decltype((self.base_))>
        {
            if constexpr (random_access_sequence<Base>) {
                auto end_cur = cur.cur;
                advance(self.base_, end_cur, self.chunk_sz_);
                return flux::slice(self.base_, cur.cur, std::move(end_cur));
            } else {
                return flux::take(flux::slice(self.base_, cur.cur, flux::last), self.chunk_sz_);
            }
        }

        static constexpr auto dec(auto& self, cursor_type& cur)
        {
            advance(self.base_, cur.cur, cur.missing - self.chunk_sz_);
            cur.missing = 0;
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<Base> && sized_sequence<Base>
        {
            distance_t missing =
                (self.chunk_sz_ - flux::size(self.base_) % self.chunk_sz_) % self.chunk_sz_;
            return cursor_type{
                .cur = flux::last(self.base_),
                .missing = missing
            };
        }

        static constexpr auto size(auto& self) -> distance_t
            requires sized_sequence<Base>
        {
            auto s = flux::size(self.base_);
            return s/self.chunk_sz_ + (s % self.chunk_sz_ == 0 ? 0 : 1);
        }

        static constexpr auto distance(auto& self, cursor_type const& from, cursor_type const& to)
            -> distance_t
            requires random_access_sequence<Base>
        {
            return (flux::distance(self.base_, from.cur, to.cur) - from.missing + to.missing)/self.chunk_sz_;
        }

        static constexpr auto inc(auto& self, cursor_type& cur, distance_t offset) -> void
            requires random_access_sequence<Base>
        {
            if (offset > 0) {
                cur.missing = advance(self.base_, cur.cur, num::checked_mul(offset, self.chunk_sz_)) % self.chunk_sz_;
            } else if (offset < 0) {
                advance(self.base_, cur.cur, num::checked_add(num::checked_mul(offset, self.chunk_sz_), cur.missing));
                cur.missing = 0;
            }
        }
    };
};

struct chunk_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, std::integral auto chunk_sz) const
        -> sequence auto
    {
        FLUX_ASSERT(chunk_sz > 0);
        return chunk_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq),
                                                checked_cast<distance_t>(chunk_sz));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto chunk = detail::chunk_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::chunk(std::integral auto chunk_sz) &&
{
    return flux::chunk(std::move(derived()), chunk_sz);
}

} // namespace flux

#endif // FLUX_OP_CHUNK_HPP_INCLUDED


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CHUNK_BY_HPP_INCLUDED
#define FLUX_OP_CHUNK_BY_HPP_INCLUDED




namespace flux {

namespace detail {

template <multipass_sequence Base, typename Pred>
struct chunk_by_adaptor : inline_sequence_base<chunk_by_adaptor<Base, Pred>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;

public:
    constexpr explicit chunk_by_adaptor(decays_to<Base> auto&& base, Pred&& pred)
        : base_(FLUX_FWD(base)),
          pred_(std::move(pred))
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> from;
            cursor_t<Base> to;

            friend constexpr auto operator==(cursor_type const& lhs, cursor_type const& rhs) -> bool
            {
                return lhs.from == rhs.from;
            }
        };

        static constexpr auto find_next(auto& self, cursor_t<Base> cur) -> cursor_t<Base>
        {
            if (flux::is_last(self.base_, cur)) {
                return cur;
            }

            auto nxt = flux::next(self.base_, cur);

            while (!flux::is_last(self.base_, nxt)) {
                if (!std::invoke(self.pred_, flux::read_at(self.base_, cur), flux::read_at(self.base_, nxt))) {
                    break;
                }
                cur = nxt;
                flux::inc(self.base_, nxt);
            }

            return nxt;
        }

        static constexpr auto find_prev(auto& self, cursor_t<Base> cur) -> cursor_t<Base>
        {
            auto const fst = flux::first(self.base_);

            if (cur == fst || flux::dec(self.base_, cur) == fst) {
                return cur;
            }

            do {
                auto prv = flux::prev(self.base_, cur);
                if (!std::invoke(self.pred_, flux::read_at(self.base_, prv), flux::read_at(self.base_, cur))) {
                    break;
                }
                cur = std::move(prv);
            } while (cur != fst);

            return cur;
        }

    public:
        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type{
                .from = flux::first(self.base_),
                .to = find_next(self, flux::first(self.base_))
            };
        }

        static constexpr auto is_last(auto&, cursor_type const& cur) -> bool
        {
            return cur.from == cur.to;
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            cur = cursor_type{
                .from = cur.to,
                .to = find_next(self, cur.to)
            };
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(flux::slice(self.base_, cur.from, cur.to))
        {
            return flux::slice(self.base_, cur.from, cur.to);
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            return cursor_type{flux::last(self.base_), flux::last(self.base_)};
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
            requires bidirectional_sequence<Base>
        {
            cur = cursor_type{
                .from = find_prev(self, cur.from),
                .to = cur.from
            };
        }
    };
};

struct chunk_by_fn {
    template <adaptable_sequence Seq, std::move_constructible Pred>
        requires multipass_sequence<Seq> &&
                 std::predicate<Pred&, element_t<Seq>, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred) const -> multipass_sequence auto
    {
        return chunk_by_adaptor<std::decay_t<Seq>, Pred>(FLUX_FWD(seq), std::move(pred));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto chunk_by = detail::chunk_by_fn{};

template <typename Derived>
template <typename Pred>
    requires multipass_sequence<Derived> &&
             std::predicate<Pred&, element_t<Derived>, element_t<Derived>>
constexpr auto inline_sequence_base<Derived>::chunk_by(Pred pred) &&
{
    return flux::chunk_by(std::move(derived()), std::move(pred));
}

} // namespace flux

#endif


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_COMPARE_HPP_INCLUDED
#define FLUX_OP_COMPARE_HPP_INCLUDED



#include <compare>
#include <cstring>
#include <bit>

namespace flux {

namespace detail {

struct compare_fn {
private:
    template <typename Seq1, typename Seq2, typename Cmp>
    static constexpr auto impl(Seq1& seq1, Seq2& seq2, Cmp& cmp)
        -> std::decay_t<
            std::invoke_result_t<Cmp &, element_t<Seq1>, element_t<Seq2>>>
    {
        auto cur1 = flux::first(seq1);
        auto cur2 = flux::first(seq2);

        while (!flux::is_last(seq1, cur1) && !flux::is_last(seq2, cur2)) {
            if (auto r = std::invoke(cmp, flux::read_at(seq1, cur1), flux::read_at(seq2, cur2));
                r != 0) {
                return r;
            }
            flux::inc(seq1, cur1);
            flux::inc(seq2, cur2);
        }

        return !flux::is_last(seq1, cur1) ? std::strong_ordering::greater :
               !flux::is_last(seq2, cur2) ? std::strong_ordering::less :
                                            std::strong_ordering::equal;
    }

public:
    template <sequence Seq1, sequence Seq2, typename Cmp = std::compare_three_way>
        requires ordering_invocable<Cmp&, element_t<Seq1>, element_t<Seq2>>
    constexpr auto operator()(Seq1 &&seq1, Seq2 &&seq2, Cmp cmp = {}) const
        -> std::decay_t<
            std::invoke_result_t<Cmp&, element_t<Seq1>, element_t<Seq2>>>
    {
        constexpr bool can_memcmp = 
            std::same_as<Cmp, std::compare_three_way> &&
            contiguous_sequence<Seq1> && 
            contiguous_sequence<Seq2> &&
            sized_sequence<Seq1> && 
            sized_sequence<Seq2> &&
            std::same_as<value_t<Seq1>, value_t<Seq2>> &&
            std::unsigned_integral<value_t<Seq1>> &&
            ((sizeof(value_t<Seq1>) == 1) || (std::endian::native == std::endian::big));

        if constexpr (can_memcmp) {
            if (std::is_constant_evaluated()) {
                return impl(seq1, seq2, cmp);
            } else {
                auto const seq1_size = flux::usize(seq1);
                auto const seq2_size = flux::usize(seq2);
                auto min_size = std::min(seq1_size, seq2_size);

                int cmp_result = 0;
                if(min_size > 0) {
                    auto data1 = flux::data(seq1);
                    FLUX_ASSERT(data1 != nullptr);
                    auto data2 = flux::data(seq2);
                    FLUX_ASSERT(data2 != nullptr);

                    cmp_result = std::memcmp(data1, data2, min_size);
                }

                if (cmp_result == 0) {
                    if (seq1_size == seq2_size) {
                        return std::strong_ordering::equal;
                    } else if (seq1_size < seq2_size) {
                        return std::strong_ordering::less;
                    } else /* seq1_size > seq2_size */ {
                        return std::strong_ordering::greater;
                    }
                } else if (cmp_result > 0) {
                    return std::strong_ordering::greater;
                } else /* cmp_result < 0 */ {
                    return std::strong_ordering::less;
                }
            }
        } else {
            return impl(seq1, seq2, cmp);
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto compare = detail::compare_fn{};

} // namespace flux

#endif // FLUX_OP_EQUAL_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CONTAINS_HPP_INCLUDED
#define FLUX_OP_CONTAINS_HPP_INCLUDED



namespace flux {

namespace detail {

struct contains_fn {
    template <sequence Seq, typename Value>
        requires std::equality_comparable_with<element_t<Seq>, Value const&>
    constexpr auto operator()(Seq&& seq, Value const& value) const
        -> bool
    {
        return !flux::is_last(seq, flux::for_each_while(seq, [&](auto&& elem) {
            return FLUX_FWD(elem) != value;
        }));
    }
};


} // namespace detail

FLUX_EXPORT inline constexpr auto contains = detail::contains_fn{};

template <typename D>
template <typename Value>
    requires std::equality_comparable_with<element_t<D>, Value const&>
constexpr auto inline_sequence_base<D>::contains(Value const& value) -> bool
{
    return flux::contains(derived(), value);
}

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
    constexpr auto operator()(Seq&& seq) const -> distance_t
    {
        if constexpr (sized_sequence<Seq>) {
            return flux::size(seq);
        } else {
            distance_t counter = 0;
            flux::for_each_while(seq, [&](auto&&) {
                ++counter;
                return true;
            });
            return counter;
        }
    }
};

struct count_eq_fn {
    template <sequence Seq, typename Value>
        requires std::equality_comparable_with<element_t<Seq>, Value const&>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Value const& value) const
        -> distance_t
    {
        distance_t counter = 0;
        flux::for_each_while(seq, [&](auto&& elem) {
            if (value == FLUX_FWD(elem)) {
                ++counter;
            }
            return true;
        });
        return counter;
    }
};

struct count_if_fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred) const
        -> distance_t
    {
        distance_t counter = 0;
        flux::for_each_while(seq, [&](auto&& elem) {
            if (std::invoke(pred, FLUX_FWD(elem))) {
                ++counter;
            }
            return true;
        });
        return counter;
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto count = detail::count_fn{};
FLUX_EXPORT inline constexpr auto count_eq = detail::count_eq_fn{};
FLUX_EXPORT inline constexpr auto count_if = detail::count_if_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::count()
{
    return flux::count(derived());
}

template <typename D>
template <typename Value>
    requires std::equality_comparable_with<element_t<D>, Value const&>
constexpr auto inline_sequence_base<D>::count_eq(Value const& value)
{
    return flux::count_eq(derived(), value);
}

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::count_if(Pred pred)
{
    return flux::count_if(derived(), std::move(pred));
}

} // namespace flux

#endif


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CURSORS_HPP_INCLUDED
#define FLUX_OP_CURSORS_HPP_INCLUDED



namespace flux {

namespace detail {

template <typename Base>
struct cursors_adaptor : inline_sequence_base<cursors_adaptor<Base>> {
private:
    Base base_;

public:
    constexpr explicit cursors_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_sequence_traits : default_sequence_traits {

        static inline constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto first(auto& self) -> decltype(flux::first(self.base_))
        {
            return flux::first(self.base_);
        }

        static constexpr auto is_last(auto& self, cursor_t<Base> const& cur) -> bool
        {
            return flux::is_last(self.base_, cur);
        }

        static constexpr auto inc(auto& self, cursor_t<Base>& cur) -> void
        {
            flux::inc(self.base_, cur);
        }

        static constexpr auto read_at(auto&, cursor_t<Base> const& cur)
            -> cursor_t<Base>
        {
            return cur;
        }

        static constexpr auto dec(auto& self, cursor_t<Base>& cur) -> void
            requires bidirectional_sequence<Base>
        {
            flux::dec(self.base_, cur);
        }

        static constexpr auto inc(auto& self, cursor_t<Base>& cur, distance_t offset) -> void
            requires random_access_sequence<Base>
        {
            flux::inc(self.base_, cur, offset);
        }

        static constexpr auto distance(auto& self, cursor_t<Base> const& from,
                                       cursor_t<Base> const& to) -> distance_t
            requires random_access_sequence<Base>
        {
            return flux::distance(self.base_, from, to);
        }

        static constexpr auto last(auto& self) -> cursor_t<Base>
            requires bounded_sequence<Base>
        {
            return flux::last(self.base_);
        }

        static constexpr auto size(auto& self) -> distance_t
            requires sized_sequence<Base>
        {
            return flux::size(self.base_);
        }
    };
};

struct cursors_fn {
    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> sequence auto
    {
        return cursors_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto cursors = detail::cursors_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::cursors() &&
    requires multipass_sequence<D>
{
    return flux::cursors(std::move(derived()));
}

} // namespace flux

#endif // FLUX_OP_CURSORS_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CYCLE_HPP_INCLUDED
#define FLUX_OP_CYCLE_HPP_INCLUDED



namespace flux {

namespace detail {

template <bool>
struct cycle_data {
    std::size_t count;
};

template <>
struct cycle_data<true> {};

template <multipass_sequence Base, bool IsInfinite>
struct cycle_adaptor : inline_sequence_base<cycle_adaptor<Base, IsInfinite>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS cycle_data<IsInfinite> data_;

public:
    constexpr explicit cycle_adaptor(decays_to<Base> auto&& base)
        requires IsInfinite
        : base_(FLUX_FWD(base))
    {}

    constexpr cycle_adaptor(decays_to<Base> auto&& base, std::size_t count)
        requires (!IsInfinite)
        : base_(FLUX_FWD(base)),
          data_(count)
    {}

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;
            // Use an unsigned type to avoid UB on overflow
            std::size_t n = 0;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool = default;

            friend auto operator<=>(cursor_type const&, cursor_type const&)
                -> std::strong_ordering
                requires std::three_way_comparable<cursor_t<Base>, std::strong_ordering>
            = default;
        };

    public:
        using value_type = value_t<Base>;

        static constexpr bool is_infinite = IsInfinite;

        static constexpr auto first(auto& self)
            -> decltype(cursor_type{flux::first(self.base_)})
        {
            if constexpr (IsInfinite) {
                return cursor_type{flux::first(self.base_)};
            } else {
                auto cur = flux::first(self.base_);
                if (flux::is_last(self.base_, cur)) {
                    return cursor_type{std::move(cur), self.data_.count};
                } else {
                    return cursor_type{std::move(cur)};
                }
            }
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            if constexpr (IsInfinite) {
                return false;
            } else {
                return cur.n >= self.data_.count;
            }
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            flux::inc(self.base_, cur.base_cur);
            if (flux::is_last(self.base_, cur.base_cur)) {
                cur.base_cur = flux::first(self.base_);
                ++cur.n;
            }
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(static_cast<const_element_t<Base>>(flux::read_at(self.base_, cur.base_cur)))
        {
            return static_cast<const_element_t<Base>>(
                flux::read_at(self.base_, cur.base_cur));
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> const_element_t<Base>
        {
            return static_cast<const_element_t<Base>>(
                flux::read_at_unchecked(self.base_, cur.base_cur));
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(auto)
        {
            using R = std::common_reference_t<value_t<Base> const&&, rvalue_element_t<Base>>;
            return static_cast<R>(flux::move_at(self.base_, cur.base_cur));
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(auto)
        {
            using R = std::common_reference_t<value_t<Base> const&&, rvalue_element_t<Base>>;
            return static_cast<R>(flux::move_at_unchecked(self.base_, cur.base_cur));
        }

        static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_type
        {
            auto constify_pred = [&pred](auto&& elem) {
                return std::invoke(pred, static_cast<const_element_t<Base>>(FLUX_FWD(elem)));
            };

            if constexpr (IsInfinite) {
                std::size_t n = 0;
                while (true) {
                    auto cur = flux::for_each_while(self.base_, constify_pred);
                    if (!flux::is_last(self.base_, cur)) {
                        return cursor_type{std::move(cur), n};
                    }
                    ++n;
                }
            } else {
                for (std::size_t n = 0; n < self.data_.count; ++n) {
                    auto cur = flux::for_each_while(self.base_, constify_pred);
                    if (!flux::is_last(self.base_, cur)) {
                        return cursor_type{std::move(cur), n};
                    }
                }
                return last(self);
            }
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
            requires bidirectional_sequence<decltype(self.base_)> &&
                     bounded_sequence<decltype(self.base_)>
        {
            if (cur.base_cur == flux::first(self.base_)) {
                --cur.n;
                cur.base_cur = flux::last(self.base_);
            }
            flux::dec(self.base_, cur.base_cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur, distance_t offset)
            requires random_access_sequence<decltype(self.base_)> &&
                     bounded_sequence<decltype(self.base_)>
        {
            auto const first = flux::first(self.base_);

            auto const sz = flux::size(self.base_);
            if (sz == 0) {
                return;
            }

            auto off = flux::distance(self.base_, first, cur.base_cur);
            off = num::checked_add(off, offset);

            cur.n += static_cast<std::size_t>(off/sz);

            off = off % sz;
            if (off < 0) {
                off +=sz; // differing signs
            }

            cur.base_cur = flux::next(self.base_, first, off);
        }

        static constexpr auto distance(auto& self,
                                       cursor_type const& from,
                                       cursor_type const& to) -> distance_t
            requires random_access_sequence<decltype(self.base_)> &&
                     sized_sequence<decltype(self.base_)>
        {
            auto dist = checked_cast<distance_t>(to.n) - checked_cast<distance_t>(from.n);
            dist = num::checked_mul(dist, flux::size(self.base_));
            return num::checked_add(dist,
                    flux::distance(self.base_, from.base_cur, to.base_cur));
        }

        // Weirdly, we don't actually need Base to be bounded
        static constexpr auto last(auto& self) -> cursor_type
            requires (!IsInfinite)
        {
            return cursor_type{.base_cur = flux::first(self.base_),
                               .n = self.data_.count};
        }

        static constexpr auto size(auto& self) -> distance_t
            requires (!IsInfinite && sized_sequence<Base>)
        {
            return num::checked_mul(flux::size(self.base_),
                                    checked_cast<flux::distance_t>(self.data_.count));
        }
    };
};

struct cycle_fn {
    template <adaptable_sequence Seq>
        requires infinite_sequence<Seq> || multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> infinite_sequence auto
    {
        if constexpr (infinite_sequence<Seq>) {
            return FLUX_FWD(seq);
        } else {
            return cycle_adaptor<std::decay_t<Seq>, true>(FLUX_FWD(seq));
        }
    }

    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, std::integral auto count) const
        -> multipass_sequence auto
    {
        auto c = checked_cast<distance_t>(count);
        if (c < 0) {
            runtime_error("Negative count passed to cycle()");
        }
        return cycle_adaptor<std::decay_t<Seq>, false>(
            FLUX_FWD(seq), checked_cast<std::size_t>(c));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto cycle = detail::cycle_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::cycle() &&
    requires infinite_sequence<D> || multipass_sequence<D>
{
    return flux::cycle(std::move(derived()));
}

template <typename D>
constexpr auto inline_sequence_base<D>::cycle(std::integral auto count) &&
    requires multipass_sequence<D>
{
    return flux::cycle(std::move(derived()), count);
}

} // namespace flux

#endif // FLUX_OP_CYCLE_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_DROP_HPP_INCLUDED
#define FLUX_OP_DROP_HPP_INCLUDED





namespace flux {

namespace detail {

template <sequence Base>
struct drop_adaptor : inline_sequence_base<drop_adaptor<Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    distance_t count_;

public:
    constexpr drop_adaptor(decays_to<Base> auto&& base, distance_t count)
        : base_(FLUX_FWD(base)),
          count_(count)
    {}

    constexpr Base& base() & { return base_; }
    constexpr Base const& base() const& { return base_; }

    struct flux_sequence_traits : passthrough_traits_base<drop_adaptor> {
        using value_type = value_t<Base>;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;

        static constexpr auto first(auto& self) -> cursor_t<Base>
        {
            auto cur = flux::first(self.base_);
            detail::advance(self.base_, cur, self.count_);
            return cur;
        }

        static constexpr auto size(auto& self)
            requires sized_sequence<Base>
        {
            return (cmp::max)(num::checked_sub(flux::size(self.base()), self.count_),
                              distance_t{0});
        }

        static constexpr auto data(auto& self)
            requires contiguous_sequence<Base> && sized_sequence<Base>
        {
            return flux::data(self.base()) + (cmp::min)(self.count_, flux::size(self.base_));
        }

        static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_t<Base>
        {
            return default_sequence_traits::for_each_while(self, FLUX_FWD(pred));
        }
    };
};

struct drop_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, std::integral auto count) const
    {
        auto count_ = checked_cast<distance_t>(count);
        if (count_ < 0) {
            runtime_error("Negative argument passed to drop()");
        }

        return drop_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq), count_);
    }

};

} // namespace detail

FLUX_EXPORT inline constexpr auto drop = detail::drop_fn{};

template <typename Derived>
constexpr auto inline_sequence_base<Derived>::drop(std::integral auto count) &&
{
    return flux::drop(std::move(derived()), count);
}

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_DROP_WHILE_HPP_INCLUDED
#define FLUX_OP_DROP_WHILE_HPP_INCLUDED




namespace flux {

namespace detail {

template <sequence Base, typename Pred>
struct drop_while_adaptor : inline_sequence_base<drop_while_adaptor<Base, Pred>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;

    friend struct passthrough_traits_base<Base>;

    constexpr auto base() & -> Base& { return base_; }
    constexpr auto base() const& -> Base const& { return base_; }

public:
    constexpr drop_while_adaptor(decays_to<Base> auto&& base, decays_to<Pred> auto&& pred)
        : base_(FLUX_FWD(base)),
          pred_(FLUX_FWD(pred))
    {}

    struct flux_sequence_traits : detail::passthrough_traits_base<Base> {
        using value_type = value_t<Base>;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;

        static constexpr auto first(auto& self)
        {
            return flux::for_each_while(self.base_, std::ref(self.pred_));
        }

        static constexpr auto data(auto& self)
            requires contiguous_sequence<Base>
        {
            return flux::data(self.base_) +
                   flux::distance(self.base_, flux::first(self.base_), first(self));
        }

        using default_sequence_traits::size;
        using default_sequence_traits::for_each_while;
    };
};

struct drop_while_fn {
    template <adaptable_sequence Seq, std::move_constructible Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred) const
    {
        return drop_while_adaptor<std::decay_t<Seq>, Pred>(FLUX_FWD(seq), std::move(pred));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto drop_while = detail::drop_while_fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::drop_while(Pred pred) &&
{
    return flux::drop_while(std::move(derived()), std::move(pred));
};

} // namespace flux

#endif


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ENDS_WITH_HPP_INCLUDED
#define FLUX_OP_ENDS_WITH_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_EQUAL_HPP_INCLUDED
#define FLUX_OP_EQUAL_HPP_INCLUDED


#include <type_traits>
#include <cstring>

namespace flux {

namespace detail {

struct equal_fn {
private:
    template <typename Seq1, typename Seq2, typename Cmp>
    static constexpr auto impl(Seq1& seq1, Seq2& seq2, Cmp cmp)
    {
        auto cur1 = flux::first(seq1);
        auto cur2 = flux::first(seq2);

        while (!flux::is_last(seq1, cur1) && !flux::is_last(seq2, cur2)) {
            if (!std::invoke(cmp, flux::read_at(seq1, cur1), flux::read_at(seq2, cur2))) {
                return false;
            }
            flux::inc(seq1, cur1);
            flux::inc(seq2, cur2);
        }

        return flux::is_last(seq1, cur1) == flux::is_last(seq2, cur2);
    }

public:
    template <sequence Seq1, sequence Seq2, typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, element_t<Seq1>, element_t<Seq2>>
    constexpr auto operator()(Seq1&& seq1, Seq2&& seq2, Cmp cmp = {}) const
        -> bool
    {
        if constexpr (sized_sequence<Seq1> && sized_sequence<Seq2>) {
            if (flux::size(seq1) != flux::size(seq2)) {
                return false;
            }
        }

        constexpr bool can_memcmp = 
            std::same_as<Cmp, std::ranges::equal_to> &&
            contiguous_sequence<Seq1> && contiguous_sequence<Seq2> &&
            sized_sequence<Seq1> && sized_sequence<Seq2> &&
            std::same_as<value_t<Seq1>, value_t<Seq2>> &&
            (std::integral<value_t<Seq1>> || std::is_pointer_v<value_t<Seq1>>) &&
            std::has_unique_object_representations_v<value_t<Seq1>>;

        if constexpr (can_memcmp) {
            if (std::is_constant_evaluated()) {
                return impl(seq1, seq2, cmp);
            } else {
                auto size = flux::usize(seq1);
                if(size == 0) {
                    return true;
                }

                auto data1 = flux::data(seq1);
                auto data2 = flux::data(seq2);
                FLUX_ASSERT(data1 != nullptr);
                FLUX_ASSERT(data2 != nullptr);

                auto result = std::memcmp(data1, data2, size * sizeof(value_t<Seq1>));
                return result == 0;
            }
        } else {
            return impl(seq1, seq2, cmp);
        }
    }

    template <sequence Seq1, sequence Seq2>
        requires (sequence<element_t<Seq1>> &&
                 sequence<element_t<Seq2>> &&
                 !std::equality_comparable_with<element_t<Seq1>, element_t<Seq2>> &&
                 std::is_invocable_v<equal_fn&, Seq1&, Seq2&, equal_fn&>)
    constexpr auto operator()(Seq1&& seq1, Seq2&& seq2) const -> bool
    {
        if constexpr (sized_sequence<Seq1> && sized_sequence<Seq2>) {
            if (flux::size(seq1) != flux::size(seq2)) {
                return false;
            }
        }

        return (*this)(seq1, seq2, *this);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto equal = detail::equal_fn{};

} // namespace flux

#endif // FLUX_OP_EQUAL_HPP_INCLUDED


namespace flux {

namespace detail {

struct ends_with_fn {
private:
    template <typename H, typename N>
    static constexpr auto bidir_impl(H& h, N& n, auto& cmp) -> bool
    {
        if constexpr (sized_sequence<H> && sized_sequence<N>) {
            if (flux::size(h) < flux::size(n)) {
                return false;
            }
        }

        auto cur1 = flux::last(h);
        auto cur2 = flux::last(n);

        auto const f1 = flux::first(h);
        auto const f2 = flux::first(n);

        if (cur2 == f2) {
            return true;
        } else if (cur1 == f1) {
            return false;
        }

        while (true) {
            flux::dec(h, cur1);
            flux::dec(n, cur2);

            if (!std::invoke(cmp, flux::read_at(h, cur1), flux::read_at(n, cur2))) {
                return false;
            }

            if (cur2 == f2) {
                return true;
            } else if (cur1 == f1) {
                return false;
            }
        }
    }

public:
    template <sequence Haystack, sequence Needle, typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, element_t<Haystack>, element_t<Needle>> &&
                 (multipass_sequence<Haystack> || sized_sequence<Haystack>) &&
                 (multipass_sequence<Needle> || sized_sequence<Needle>)
    constexpr auto operator()(Haystack&& haystack, Needle&& needle, Cmp cmp = Cmp{}) const
        -> bool
    {
        if constexpr(bidirectional_sequence<Haystack> &&
                     bounded_sequence<Haystack> &&
                     bidirectional_sequence<Needle> &&
                     bounded_sequence<Needle>) {
            return bidir_impl(haystack, needle, cmp);
        } else {
            distance_t len1 = flux::count(haystack);
            distance_t len2 = flux::count(needle);

            if (len1 < len2) {
                return false;
            }

            auto cur1 = flux::first(haystack);
            detail::advance(haystack, cur1, len1 - len2);

            return flux::equal(flux::slice(haystack, std::move(cur1), flux::last),
                               needle, std::move(cmp));
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto ends_with = detail::ends_with_fn{};

template <typename Derived>
template <sequence Needle, typename Cmp>
    requires std::predicate<Cmp&, element_t<Derived>, element_t<Needle>> &&
             (multipass_sequence<Derived> || sized_sequence<Derived>) &&
             (multipass_sequence<Needle> || sized_sequence<Needle>)
constexpr auto inline_sequence_base<Derived>::ends_with(Needle&& needle, Cmp cmp) -> bool
{
    return flux::ends_with(derived(), FLUX_FWD(needle), std::move(cmp));
}


} // namespace flux

#endif // FLUX_OP_ENDS_WITH_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FILL_HPP_INCLUDED
#define FLUX_OP_FILL_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FOR_EACH_HPP_INCLUDED
#define FLUX_OP_FOR_EACH_HPP_INCLUDED



namespace flux {

namespace detail {

struct for_each_fn {

    template <sequence Seq, typename Func>
        requires (std::invocable<Func&, element_t<Seq>> &&
                  !infinite_sequence<Seq>)
    constexpr auto operator()(Seq&& seq, Func func) const -> Func
    {
        (void) flux::for_each_while(FLUX_FWD(seq), [&](auto&& elem) {
            std::invoke(func, FLUX_FWD(elem));
            return true;
        });
        return func;
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto for_each = detail::for_each_fn{};

template <typename D>
template <typename Func>
    requires std::invocable<Func&, element_t<D>>
constexpr auto inline_sequence_base<D>::for_each(Func func) -> Func
{
    return flux::for_each(derived(), std::move(func));
}

} // namespace flux

#endif

#include <type_traits>
#include <cstring>

namespace flux {

namespace detail {

struct fill_fn {
private:
    template <typename Seq, typename Value>
    static constexpr auto impl(Seq& seq, Value const& value)
    {
        flux::for_each(seq, [&value](auto&& elem) { FLUX_FWD(elem) = value; });
    }

public:
    template <typename Value, writable_sequence_of<Value> Seq>
    constexpr void operator()(Seq&& seq, Value const& value) const
    {
        constexpr bool can_memset = 
            contiguous_sequence<Seq> &&
            sized_sequence<Seq> &&
            std::same_as<Value, value_t<Seq>> &&
            // only allow memset on single byte types
            sizeof(value_t<Seq>) == 1 &&
            std::is_trivially_copyable_v<value_t<Seq>>;

        if constexpr (can_memset) {
            if (std::is_constant_evaluated()) {
                impl(seq, value);
            } else {
                auto size = flux::usize(seq);
                if(size == 0) {
                    return;
                }
                
                FLUX_ASSERT(flux::data(seq) != nullptr);
                
                std::memset(flux::data(seq), value,
                    size * sizeof(value_t<Seq>));
            }
        } else {
            impl(seq, value);
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto fill = detail::fill_fn{};

template <typename D>
template <typename Value>
    requires writable_sequence_of<D, Value const&>
constexpr void inline_sequence_base<D>::fill(Value const& value)
{
    flux::fill(derived(), value);
}

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FILTER_HPP_INCLUDED
#define FLUX_OP_FILTER_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FIND_HPP_INCLUDED
#define FLUX_OP_FIND_HPP_INCLUDED




#include <cstring>
#include <type_traits>

namespace flux {

namespace detail {

struct find_fn {
private:
    template <typename Seq, typename Value>
    static constexpr auto impl(Seq&& seq, Value const& value) -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return FLUX_FWD(elem) != value;
        });
    }

public:
    template <sequence Seq, typename Value>
        requires std::equality_comparable_with<element_t<Seq>, Value const&>
    constexpr auto operator()(Seq&& seq, Value const& value) const -> cursor_t<Seq>
    {
        constexpr auto can_memchr = 
            contiguous_sequence<Seq> && sized_sequence<Seq> && 
            std::same_as<Value, value_t<Seq>> &&
            flux::detail::any_of<value_t<Seq>, char, signed char, unsigned char, char8_t, std::byte>;

        if constexpr (can_memchr) {
            if (std::is_constant_evaluated()) {
                return impl(seq, value);
            } else {
                auto size = flux::usize(seq);
                if (size == 0) {
                    return flux::last(seq);
                }
                FLUX_ASSERT(flux::data(seq) != nullptr);
                auto location = std::memchr(flux::data(seq), static_cast<unsigned char>(value),
                    size * sizeof(value_t<Seq>));
                if (location == nullptr) {
                    return flux::last(seq);
                } else {
                    auto offset = static_cast<value_t<Seq> const*>(location) - flux::data(seq);
                    return flux::next(seq, flux::first(seq), offset);
                }
            }
        } else {
            return impl(seq, value);
        }
    }
};

struct find_if_fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr auto operator()(Seq&& seq, Pred pred) const
        -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, FLUX_FWD(elem));
        });
    }
};

struct find_if_not_fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr auto operator()(Seq&& seq, Pred pred) const
        -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return std::invoke(pred, FLUX_FWD(elem));
        });
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto find = detail::find_fn{};
FLUX_EXPORT inline constexpr auto find_if = detail::find_if_fn{};
FLUX_EXPORT inline constexpr auto find_if_not = detail::find_if_not_fn{};

template <typename D>
template <typename Value>
    requires std::equality_comparable_with<element_t<D>, Value const&>
constexpr auto inline_sequence_base<D>::find(Value const& val)
{
    return flux::find(derived(), val);
}

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::find_if(Pred pred)
{
    return flux::find_if(derived(), std::ref(pred));
}

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::find_if_not(Pred pred)
{
    return flux::find_if_not(derived(), std::ref(pred));
}

} // namespace flux

#endif // FLUX_OP_FIND_HPP_INCLUDED



namespace flux {

namespace detail {

template <sequence Base, typename Pred>
class filter_adaptor : public inline_sequence_base<filter_adaptor<Base, Pred>>
{
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;

public:
    constexpr filter_adaptor(decays_to<Base> auto&& base, decays_to<Pred> auto&& pred)
        : base_(FLUX_FWD(base)),
          pred_(FLUX_FWD(pred))
    {}

    [[nodiscard]]
    constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]]
    constexpr auto base() && -> Base { return std::move(base_); }

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool
                requires std::equality_comparable<cursor_t<Base>>
                = default;
        };

    public:
        using value_type = value_t<Base>;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;

        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type{flux::find_if(self.base_, std::ref(self.pred_))};
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.base_cur);
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at(self.base_, cur.base_cur))
        {
            return flux::read_at(self.base_, cur.base_cur);
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at_unchecked(self.base_, cur.base_cur))
        {
            return flux::read_at_unchecked(self.base_, cur.base_cur);
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at(self.base_, cur.base_cur))
        {
            return flux::move_at(self.base_, cur.base_cur);
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at_unchecked(self.base_, cur.base_cur))
        {
            return flux::move_at_unchecked(self.base_, cur.base_cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            flux::inc(self.base_, cur.base_cur);
            cur.base_cur = flux::slice(self.base_, std::move(cur).base_cur, flux::last)
                               .find_if(std::ref(self.pred_));
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
            requires bidirectional_sequence<Base>
        {
            do {
                flux::dec(self.base_, cur.base_cur);
            } while(!std::invoke(self.pred_, flux::read_at(self.base_, cur.base_cur)));
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            return cursor_type{flux::last(self.base_)};
        }

        static constexpr auto for_each_while(auto& self, auto&& func)
            -> cursor_type
        {
            return cursor_type{flux::for_each_while(self.base_, [&](auto&& elem) {
                if (std::invoke(self.pred_, elem)) {
                    return std::invoke(func, FLUX_FWD(elem));
                } else {
                    return true;
                }
            })};
        }
    };
};

struct filter_fn {
    template <adaptable_sequence Seq, std::move_constructible Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred) const
    {
        return filter_adaptor<std::decay_t<Seq>, Pred>(FLUX_FWD(seq), std::move(pred));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto filter = detail::filter_fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::filter(Pred pred) &&
{
    return detail::filter_adaptor<D, Pred>(std::move(derived()), std::move(pred));
}


} // namespace flux

#endif // FLUX_OP_FILTER_HPP_INCLUDED



// Copyright (c) 2024 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FILTER_MAP_HPP_INCLUDED
#define FLUX_OP_FILTER_MAP_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_MAP_HPP_INCLUDED
#define FLUX_OP_MAP_HPP_INCLUDED




namespace flux {

namespace detail {

template <sequence Base, typename Func>
    requires std::is_object_v<Func> &&
             std::regular_invocable<Func&, element_t<Base>>
struct map_adaptor : inline_sequence_base<map_adaptor<Base, Func>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;

    friend struct sequence_traits<map_adaptor>;

public:
    constexpr map_adaptor(decays_to<Base> auto&& base, decays_to<Func> auto&& func)
        : base_(FLUX_FWD(base)),
          func_(FLUX_FWD(func))
    {}

    constexpr auto base() & -> Base& { return base_; }
    constexpr auto base() const& -> Base const& { return base_; }
    constexpr auto base() && -> Base&& { return std::move(base_); }
    constexpr auto base() const&& -> Base const&& { return std::move(base_); }

    struct flux_sequence_traits  : detail::passthrough_traits_base<Base>
    {
        using value_type = std::remove_cvref_t<std::invoke_result_t<Func&, element_t<Base>>>;

        static constexpr bool disable_multipass = !multipass_sequence<Base>;
        static constexpr bool is_infinite = infinite_sequence<Base>;

        template <typename Self>
        static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
            -> decltype(std::invoke(self.func_, flux::read_at(self.base_, cur)))
        {
            return std::invoke(self.func_, flux::read_at(self.base_, cur));
        }

        template <typename Self>
        static constexpr auto read_at_unchecked(Self& self, cursor_t<Self> const& cur)
            -> decltype(std::invoke(self.func_, flux::read_at_unchecked(self.base_, cur)))
        {
            return std::invoke(self.func_, flux::read_at_unchecked(self.base_, cur));
        }

        static constexpr auto for_each_while(auto& self, auto&& pred)
        {
            return flux::for_each_while(self.base_, [&](auto&& elem) {
                return std::invoke(pred, std::invoke(self.func_, FLUX_FWD(elem)));
            });
        }

        using default_sequence_traits::move_at;
        using default_sequence_traits::move_at_unchecked;

        static void data() = delete; // we're not a contiguous sequence
    };
};

struct map_fn {
    template <adaptable_sequence Seq, typename Func>
        requires std::regular_invocable<Func&, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func) const
    {
        return map_adaptor<std::decay_t<Seq>, Func>(FLUX_FWD(seq), std::move(func));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto map = detail::map_fn{};

template <typename Derived>
template <typename Func>
    requires std::invocable<Func&, element_t<Derived>>
constexpr auto inline_sequence_base<Derived>::map(Func func) &&
{
    return detail::map_adaptor<Derived, Func>(std::move(derived()), std::move(func));
}

} // namespace flux

#endif


namespace flux {

namespace detail {

struct filter_map_fn {
    // If dereffing the optional would give us an rvalue reference,
    // prevent a probable dangling reference by returning by value instead
    template <typename T>
    using strip_rvalue_ref_t = std::conditional_t<
        std::is_rvalue_reference_v<T>, std::remove_reference_t<T>, T>;

    template <adaptable_sequence Seq, typename Func>
        requires (std::invocable<Func&, element_t<Seq>> &&
                  optional_like<std::remove_cvref_t<std::invoke_result_t<Func&, element_t<Seq>>>>)
    constexpr auto operator()(Seq&& seq, Func func) const
    {
        return flux::map(FLUX_FWD(seq), std::move(func))
            .filter([](auto&& opt) { return static_cast<bool>(opt); })
            .map([](auto&& opt) -> strip_rvalue_ref_t<decltype(*FLUX_FWD(opt))> {
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
    template <adaptable_sequence Seq>
        requires optional_like<value_t<Seq>>
    constexpr auto operator()(Seq&& seq) const
    {
        return filter_map(FLUX_FWD(seq), [](auto&& opt) -> decltype(auto) { return FLUX_FWD(opt); });
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

#endif



// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FIND_MIN_MAX_HPP_INCLUDED
#define FLUX_OP_FIND_MIN_MAX_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_MINMAX_HPP_INCLUDED
#define FLUX_OP_MINMAX_HPP_INCLUDED




// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FOLD_HPP_INCLUDED
#define FLUX_OP_FOLD_HPP_INCLUDED



namespace flux {

namespace detail {

template <typename Seq, typename Func, typename Init>
using fold_result_t = std::decay_t<std::invoke_result_t<Func&, Init, element_t<Seq>>>;

struct fold_op {
    template <sequence Seq, typename Func, std::movable Init = value_t<Seq>,
              typename R = fold_result_t<Seq, Func, Init>>
        requires std::invocable<Func&,  Init, element_t<Seq>> &&
                 std::invocable<Func&, R, element_t<Seq>> &&
                 std::convertible_to<Init, R> &&
                 std::assignable_from<Init&, std::invoke_result_t<Func&, R, element_t<Seq>>>
    constexpr auto operator()(Seq&& seq, Func func, Init init = Init{}) const -> R
    {
        R init_ = R(std::move(init));
        flux::for_each_while(seq, [&func, &init_](auto&& elem) {
            init_ = std::invoke(func, std::move(init_), FLUX_FWD(elem));
            return true;
        });
        return init_;
    }
};

struct fold_first_op {
    template <sequence Seq, typename Func, typename V = value_t<Seq>>
        requires std::invocable<Func&, V, element_t<Seq>> &&
                 std::assignable_from<value_t<Seq>&, std::invoke_result_t<Func&, V&&, element_t<Seq>>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func) const -> flux::optional<V>
    {
        auto cur = flux::first(seq);

        if (flux::is_last(seq, cur)) {
            return std::nullopt;
        }

        V init(flux::read_at(seq, cur));
        flux::inc(seq, cur);

        while (!flux::is_last(seq, cur)) {
            init = std::invoke(func, std::move(init), flux::read_at(seq, cur));
            flux::inc(seq, cur);
        }

        return flux::optional<V>(std::in_place, std::move(init));
    }
};

struct sum_op {
    template <sequence Seq>
        requires std::default_initializable<value_t<Seq>> &&
                 std::invocable<fold_op, Seq, std::plus<>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> value_t<Seq>
    {
        return fold_op{}(FLUX_FWD(seq), std::plus<>{}, value_t<Seq>(0));
    }
};

struct product_op {
    template <sequence Seq>
        requires std::invocable<fold_op, Seq, std::multiplies<>> &&
                 requires { value_t<Seq>(1); }
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> value_t<Seq>
    {
        return fold_op{}(FLUX_FWD(seq), std::multiplies<>{}, value_t<Seq>(1));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto fold = detail::fold_op{};
FLUX_EXPORT inline constexpr auto fold_first = detail::fold_first_op{};
FLUX_EXPORT inline constexpr auto sum = detail::sum_op{};
FLUX_EXPORT inline constexpr auto product = detail::product_op{};

template <typename Derived>
template <typename D, typename Func, typename Init>
    requires foldable<Derived, Func, Init>
[[nodiscard]]
constexpr auto inline_sequence_base<Derived>::fold(Func func, Init init) -> fold_result_t<D, Func, Init>
{
    return flux::fold(derived(), std::move(func), std::move(init));
}

template <typename Derived>
template <typename D, typename Func>
    requires std::invocable<Func&, value_t<D>, element_t<D>> &&
             std::assignable_from<value_t<D>&, std::invoke_result_t<Func&, value_t<D>, element_t<D>>>
constexpr auto inline_sequence_base<Derived>::fold_first(Func func)
{
    return flux::fold_first(derived(), std::move(func));
}

template <typename D>
constexpr auto inline_sequence_base<D>::sum()
    requires foldable<D, std::plus<>, value_t<D>> &&
             std::default_initializable<value_t<D>>
{
    return flux::sum(derived());
}

template <typename D>
constexpr auto inline_sequence_base<D>::product()
    requires foldable<D, std::multiplies<>, value_t<D>> &&
             requires { value_t<D>(1); }
{
    return flux::product(derived());
}

} // namespace flux

#endif // FLUX_OP_FOLD_HPP_INCLUDED



namespace flux {

FLUX_EXPORT
template <typename T>
struct minmax_result {
    T min;
    T max;
};

namespace detail {

struct min_op {
    template <sequence Seq, weak_ordering_for<Seq> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Cmp cmp = Cmp{}) const
        -> flux::optional<value_t<Seq>>
    {
        return flux::fold_first(FLUX_FWD(seq), [&](auto min, auto&& elem) -> value_t<Seq> {
            if (std::invoke(cmp, elem, min) < 0) {
                return value_t<Seq>(FLUX_FWD(elem));
            } else {
                return min;
            }
        });
    }
};

struct max_op {
    template <sequence Seq, weak_ordering_for<Seq> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Cmp cmp = Cmp{}) const
        -> flux::optional<value_t<Seq>>
    {
        return flux::fold_first(FLUX_FWD(seq), [&](auto max, auto&& elem) -> value_t<Seq> {
            if (!(std::invoke(cmp, elem, max) < 0)) {
                return value_t<Seq>(FLUX_FWD(elem));
            } else {
                return max;
            }
        });
    }
};

struct minmax_op {
    template <sequence Seq, weak_ordering_for<Seq> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Cmp cmp = Cmp{}) const
        -> flux::optional<minmax_result<value_t<Seq>>>
    {
        using R = minmax_result<value_t<Seq>>;

        auto cur = flux::first(seq);
        if (flux::is_last(seq, cur)) {
            return std::nullopt;
        }

        R init = R{value_t<Seq>(flux::read_at(seq, cur)),
                   value_t<Seq>(flux::read_at(seq, cur))};

        auto fold_fn = [&](R mm, auto&& elem) -> R {
            if (std::invoke(cmp, elem, mm.min) < 0) {
                mm.min = value_t<Seq>(elem);
            }
            if (!(std::invoke(cmp, elem, mm.max) < 0)) {
                mm.max = value_t<Seq>(FLUX_FWD(elem));
            }
            return mm;
        };

        return flux::optional<R>(std::in_place,
                                flux::fold(flux::slice(seq, std::move(cur), flux::last), fold_fn, std::move(init)));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto min = detail::min_op{};
FLUX_EXPORT inline constexpr auto max = detail::max_op{};
FLUX_EXPORT inline constexpr auto minmax = detail::minmax_op{};

template <typename Derived>
template <typename Cmp>
    requires weak_ordering_for<Cmp, Derived>
constexpr auto inline_sequence_base<Derived>::max(Cmp cmp)
{
    return flux::max(derived(), std::move(cmp));
}

template <typename Derived>
template <typename Cmp>
    requires weak_ordering_for<Cmp, Derived>
constexpr auto inline_sequence_base<Derived>::min(Cmp cmp)
{
    return flux::min(derived(), std::move(cmp));
}

template <typename Derived>
template <typename Cmp>
    requires weak_ordering_for<Cmp, Derived>
constexpr auto inline_sequence_base<Derived>::minmax(Cmp cmp)
{
    return flux::minmax(derived(), std::move(cmp));
}

} // namespace flux

#endif // FLUX_OP_MINMAX_HPP_INCLUDED


namespace flux {

namespace detail {

struct find_min_fn {
    template <multipass_sequence Seq,
              weak_ordering_for<Seq> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Cmp cmp = {}) const -> cursor_t<Seq>
    {
        auto min = first(seq);
        if (!is_last(seq, min)) {
            for (auto cur = next(seq, min); !is_last(seq, cur); inc(seq, cur)) {
                if (std::invoke(cmp, read_at(seq, cur), read_at(seq, min)) < 0) {
                    min = cur;
                }
            }
        }

        return min;
    }
};

struct find_max_fn {
    template <multipass_sequence Seq,
              weak_ordering_for<Seq> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Cmp cmp = {}) const -> cursor_t<Seq>
    {
        auto max = first(seq);
        if (!is_last(seq, max)) {
            for (auto cur = next(seq, max); !is_last(seq, cur); inc(seq, cur)) {
                if (!(std::invoke(cmp, read_at(seq, cur), read_at(seq, max)) < 0)) {
                    max = cur;
                }
            }
        }

        return max;
    }
};

struct find_minmax_fn {
    template <multipass_sequence Seq,
              weak_ordering_for<Seq> Cmp = std::compare_three_way>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Cmp cmp = {}) const
        -> minmax_result<cursor_t<Seq>>
    {
        auto min = first(seq);
        auto max = min;
        if (!is_last(seq, min)) {
            for (auto cur = next(seq, min); !is_last(seq, cur); inc(seq, cur)) {
                auto&& elem = read_at(seq, cur);

                if (std::invoke(cmp, elem, read_at(seq, min)) < 0) {
                    min = cur;
                }
                if (!(std::invoke(cmp, elem, read_at(seq, max)) < 0)) {
                    max = cur;
                }
            }
        }

        return {std::move(min), std::move(max)};
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto find_min = detail::find_min_fn{};
FLUX_EXPORT inline constexpr auto find_max = detail::find_max_fn{};
FLUX_EXPORT inline constexpr auto find_minmax = detail::find_minmax_fn{};

template <typename D>
template <typename Cmp>
    requires weak_ordering_for<Cmp, D>
constexpr auto inline_sequence_base<D>::find_min(Cmp cmp)
{
    return flux::find_min(derived(), std::move(cmp));
}

template <typename D>
template <typename Cmp>
    requires weak_ordering_for<Cmp, D>
constexpr auto inline_sequence_base<D>::find_max(Cmp cmp)
{
    return flux::find_max(derived(), std::move(cmp));
}

template <typename D>
template <typename Cmp>
    requires weak_ordering_for<Cmp, D>
constexpr auto inline_sequence_base<D>::find_minmax(Cmp cmp)
{
    return flux::find_minmax(derived(), std::move(cmp));
}

} // namespace flux

#endif // FLUX_OP_FIND_MIN_MAX_HPP_INCLUDED


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FLATTEN_HPP_INCLUDED
#define FLUX_OP_FLATTEN_HPP_INCLUDED



namespace flux {

namespace detail {

template <sequence Base>
struct flatten_adaptor : inline_sequence_base<flatten_adaptor<Base>> {
private:
    using InnerSeq = element_t<Base>;

    Base base_;
    optional<InnerSeq> inner_ = nullopt;

public:
    constexpr explicit flatten_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        using self_t = flatten_adaptor;

        struct cursor_type {
            constexpr explicit cursor_type(cursor_t<Base>&& outer_cur)
                : outer_cur(std::move(outer_cur))
            {}

            cursor_type() = default;
            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;

            cursor_t<Base> outer_cur;
            optional<cursor_t<InnerSeq>> inner_cur = nullopt;
        };

        static constexpr auto satisfy(auto& self, cursor_type& cur) -> void
        {
            while (!flux::is_last(self.base_, cur.outer_cur)) {
                self.inner_.emplace(flux::read_at(self.base_, cur.outer_cur));
                cur.inner_cur.emplace(flux::first(*self.inner_));
                if (!flux::is_last(*self.inner_, *cur.inner_cur)) {
                    return;
                }
                flux::inc(self.base_, cur.outer_cur);
            }
        }

    public:
        using value_type = value_t<InnerSeq>;

        static constexpr auto first(self_t& self) -> cursor_type
        {
            cursor_type cur(flux::first(self.base_));
            satisfy(self, cur);
            return cur;
        }

        static constexpr auto is_last(self_t& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.outer_cur);
        }

        static constexpr auto inc(self_t& self, cursor_type& cur) -> void
        {
            flux::inc(*self.inner_, *cur.inner_cur);
            if (flux::is_last(*self.inner_, *cur.inner_cur)) {
                flux::inc(self.base_, cur.outer_cur);
                satisfy(self, cur);
            }
        }

        static constexpr auto read_at(self_t& self, cursor_type const& cur) -> decltype(auto)
        {
            FLUX_ASSERT(self.inner_.has_value());
            FLUX_ASSERT(cur.inner_cur.has_value());
            return flux::read_at(*self.inner_, *cur.inner_cur);
        }

        static constexpr auto last(self_t& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            return cursor_type(flux::last(self.base_));
        }
    };
};

template <multipass_sequence Base>
    requires std::is_reference_v<element_t<Base>> &&
             multipass_sequence<element_t<Base>>
struct flatten_adaptor<Base> : inline_sequence_base<flatten_adaptor<Base>> {
private:
    Base base_;

public:
    constexpr explicit flatten_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        using InnerSeq = element_t<Base>;

        template <typename Self>
        static constexpr bool can_flatten = [] () consteval {
            if constexpr (std::is_const_v<Self>) {
                return multipass_sequence<Base const> &&
                       std::same_as<element_t<Base const>, std::remove_reference_t<InnerSeq> const&> &&
                       multipass_sequence<InnerSeq const>;
            } else {
                return true;
            }
        }();

        struct cursor_type {
            cursor_t<Base> outer_cur{};
            cursor_t<InnerSeq> inner_cur{};

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool = default;
        };

        static constexpr auto satisfy(auto& self, cursor_type& cur) -> void
        {
            while (true) {
                if (flux::is_last(self.base_, cur.outer_cur)) {
                    cur.inner_cur = cursor_t<InnerSeq>{};
                    return;
                }
                auto&& inner = flux::read_at(self.base_, cur.outer_cur);
                cur.inner_cur = flux::first(inner);
                if (!flux::is_last(inner, cur.inner_cur)) {
                    return;
                }
                flux::inc(self.base_, cur.outer_cur);
            }
        }

    public:
        using value_type = value_t<InnerSeq>;

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto first(Self& self) -> cursor_type
        {
            cursor_type cur{.outer_cur = flux::first(self.base_) };
            satisfy(self, cur);
            return cur;
        }

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.outer_cur);
        }

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur) -> decltype(auto)
        {
            return flux::read_at(flux::read_at(self.base_, cur.outer_cur),
                                 cur.inner_cur);
        }

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto inc(Self& self, cursor_type& cur) -> void
        {
            auto&& inner = flux::read_at(self.base_, cur.outer_cur);
            flux::inc(inner, cur.inner_cur);
            if (flux::is_last(inner, cur.inner_cur)) {
                flux::inc(self.base_, cur.outer_cur);
                satisfy(self, cur);
            }
        }

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto for_each_while(Self& self, auto&& pred) -> cursor_type
        {
            auto inner_cur = cursor_t<InnerSeq>{};
            auto outer_cur = flux::for_each_while(self.base_, [&](auto&& inner_seq) {
                inner_cur = flux::for_each_while(inner_seq, pred);
                return flux::is_last(inner_seq, inner_cur);
            });
            return cursor_type{.outer_cur = std::move(outer_cur),
                               .inner_cur = std::move(inner_cur)};
        }

        template <typename Self>
            requires can_flatten<Self> && bounded_sequence<Base>
        static constexpr auto last(Self& self) -> cursor_type
        {
            return cursor_type{.outer_cur = flux::last(self.base_)};
        }

        template <typename Self>
            requires can_flatten<Self> &&
                     bidirectional_sequence<Base> &&
                     bidirectional_sequence<InnerSeq> &&
                     bounded_sequence<InnerSeq>
        static constexpr auto dec(Self& self, cursor_type& cur) -> void
        {
            if (flux::is_last(self.base_, cur.outer_cur)) {
                flux::dec(self.base_, cur.outer_cur);
                auto&& inner = flux::read_at(self.base_, cur.outer_cur);
                cur.inner_cur = flux::last(inner);
            }
            while (true) {
                auto&& inner = flux::read_at(self.base_, cur.outer_cur);
                if (cur.inner_cur != flux::first(inner)) {
                    flux::dec(inner, cur.inner_cur);
                    return;
                } else {
                    flux::dec(self.base_, cur.outer_cur);
                    auto&& next_inner = flux::read_at(self.base_, cur.outer_cur);
                    cur.inner_cur = flux::last(next_inner);
                }
            }
        }
    };

};

struct flatten_fn {
    template <adaptable_sequence Seq>
        requires sequence<element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> sequence auto
    {
        return flatten_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto flatten = detail::flatten_fn{};

template <typename Derived>
constexpr auto inline_sequence_base<Derived>::flatten() &&
        requires sequence<element_t<Derived>>
{
    return flux::flatten(std::move(derived()));
}

} // namespace flux

#endif // FLUX_OP_FLATTEN_HPP_INCLUDED


// Copyright (c) 2024 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FLATTEN_WITH_HPP_INCLUDED
#define FLUX_OP_FLATTEN_WITH_HPP_INCLUDED




// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_SINGLE_HPP_INCLUDED
#define FLUX_SOURCE_SINGLE_HPP_INCLUDED



namespace flux {

namespace detail {

template <std::movable T>
struct single_sequence : inline_sequence_base<single_sequence<T>> {
private:
    T obj_;

    friend struct sequence_traits<single_sequence>;

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
struct sequence_traits<detail::single_sequence<T>> : default_sequence_traits
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

    static constexpr auto read_at(auto& self, [[maybe_unused]] cursor_type cur) -> auto&
    {
        FLUX_DEBUG_ASSERT(cur == cursor_type::valid);
        return self.obj_;
    }

    static constexpr auto inc(self_t const&, cursor_type& cur) -> cursor_type&
    {
        FLUX_DEBUG_ASSERT(cur == cursor_type::valid);
        cur = cursor_type::done;
        return cur;
    }

    static constexpr auto dec(self_t const&, cursor_type& cur) -> cursor_type&
    {
        FLUX_DEBUG_ASSERT(cur == cursor_type::done);
        cur = cursor_type::valid;
        return cur;
    }

    static constexpr auto inc(self_t const&, cursor_type& cur, distance_t off)
        -> cursor_type&
    {
        if (off > 0) {
            FLUX_DEBUG_ASSERT(cur == cursor_type::valid && off == 1);
            cur = cursor_type::done;
        } else if (off < 0) {
            FLUX_DEBUG_ASSERT(cur == cursor_type::done && off == -1);
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

FLUX_EXPORT inline constexpr auto single = detail::single_fn{};

} // namespace flux

#endif // FLUX_SOURCE_SINGLE_HPP_INCLUDED


namespace flux {

namespace detail {

// Workaround for std::variant::emplace<N> not being constexpr in libc++
// See P2231 (C++20 DR)
template <std::size_t N>
inline constexpr auto variant_emplace =
[]<typename... Types>(std::variant<Types...>& variant, auto&&... args) {
    if constexpr (__cpp_lib_variant >= 202106L) {
        variant.template emplace<N>(FLUX_FWD(args)...);
    } else {
        if (std::is_constant_evaluated()) {
            variant = std::variant<Types...>(std::in_place_index<N>, FLUX_FWD(args)...);
        } else {
            variant.template emplace<N>(FLUX_FWD(args)...);
        }
    }
};

template <sequence Base, multipass_sequence Pattern>
struct flatten_with_adaptor : inline_sequence_base<flatten_with_adaptor<Base, Pattern>>
{
private:
    using InnerSeq = element_t<Base>;

    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pattern pattern_;
    optional<InnerSeq> inner_ = nullopt;

public:
    constexpr flatten_with_adaptor(decays_to<Base> auto&& base,
                                   decays_to<Pattern> auto&& pattern)
        : base_(FLUX_FWD(base)),
          pattern_(FLUX_FWD(pattern))
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        using self_t = flatten_with_adaptor;
        using element_type =
            std::common_reference_t<element_t<InnerSeq>, element_t<Pattern>>;
        using rvalue_element_type =
            std::common_reference_t<rvalue_element_t<InnerSeq>, rvalue_element_t<Pattern>>;

        struct cursor_type {
            constexpr explicit cursor_type(cursor_t<Base>&& outer_cur)
                : outer_cur(std::move(outer_cur))
            {}

            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;

            cursor_t<Base> outer_cur;
            std::variant<cursor_t<Pattern>, cursor_t<InnerSeq>> inner_cur{};
        };

        static constexpr auto satisfy(self_t& self, cursor_type& cur) -> void
        {
            while (true) {
                if (cur.inner_cur.index() == 0) {
                    if (!flux::is_last(self.pattern_, std::get<0>(cur.inner_cur))) {
                        break;
                    }

                    self.inner_.emplace(flux::read_at(self.base_, cur.outer_cur));
                    variant_emplace<1>(cur.inner_cur, flux::first(*self.inner_));
                } else {
                    FLUX_ASSERT(self.inner_.has_value());
                    if (!flux::is_last(*self.inner_, std::get<1>(cur.inner_cur))) {
                        break;
                    }

                    flux::inc(self.base_, cur.outer_cur);
                    if (!flux::is_last(self.base_, cur.outer_cur)) {
                        variant_emplace<0>(cur.inner_cur, flux::first(self.pattern_));
                    } else {
                        break;
                    }
                }
            }
        }

    public:
        using value_type = std::common_type_t<value_t<InnerSeq>, value_t<Pattern>>;

        static constexpr auto first(self_t& self) -> cursor_type
        {
            cursor_type cur(flux::first(self.base_));
            if (!flux::is_last(self.base_, cur.outer_cur)) {
                self.inner_.emplace(flux::read_at(self.base_, cur.outer_cur));
                variant_emplace<1>(cur.inner_cur, flux::first(*self.inner_));
                satisfy(self, cur);
            }
            return cur;
        }

        static constexpr auto is_last(self_t& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.outer_cur);
        }

        static constexpr auto inc(self_t& self, cursor_type& cur) -> void
        {
            if (cur.inner_cur.index() == 0) {
                flux::inc(self.pattern_, std::get<0>(cur.inner_cur));
            } else {
                FLUX_ASSERT(self.inner_.has_value());
                flux::inc(*self.inner_, std::get<1>(cur.inner_cur));
            }
            satisfy(self, cur);
        }

        static constexpr auto read_at(self_t& self, cursor_type const& cur)
            -> element_type
        {
            if (cur.inner_cur.index() == 0) {
                return static_cast<element_type>(
                    flux::read_at(self.pattern_, std::get<0>(cur.inner_cur)));
            } else {
                FLUX_ASSERT(self.inner_.has_value());
                return static_cast<element_type>(
                    flux::read_at(*self.inner_, std::get<1>(cur.inner_cur)));
            }
        }

        static constexpr auto move_at(self_t& self, cursor_type const& cur)
            -> rvalue_element_type
        {
            if (cur.inner_cur.index() == 0) {
                return static_cast<rvalue_element_type>(
                    flux::move_at(self.pattern_, std::get<0>(cur.inner_cur)));
            } else {
                FLUX_ASSERT(self.inner_.has_value());
                return static_cast<rvalue_element_type>(
                    flux::move_at(*self.inner_, std::get<1>(cur.inner_cur)));
            }
        }

        static constexpr auto last(self_t& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            return cursor_type{.outer_cur = flux::last(self)};
        }
    };
};

template <multipass_sequence Base, multipass_sequence Pattern>
    requires std::is_lvalue_reference_v<element_t<Base>> &&
             multipass_sequence<element_t<Base>>
struct flatten_with_adaptor<Base, Pattern>
    : inline_sequence_base<flatten_with_adaptor<Base, Pattern>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pattern pattern_;

public:
    constexpr flatten_with_adaptor(decays_to<Base> auto&& base,
                                   decays_to<Pattern> auto&& pattern)
        : base_(FLUX_FWD(base)),
          pattern_(FLUX_FWD(pattern))
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        using InnerSeq = element_t<Base>;

        template <typename Self>
        static constexpr bool can_flatten = [] () consteval {
            if constexpr (std::is_const_v<Self>) {
                return multipass_sequence<Base const> &&
                       std::same_as<element_t<Base const>, std::remove_reference_t<InnerSeq> const&> &&
                       multipass_sequence<InnerSeq const> &&
                       multipass_sequence<Pattern const>;
            } else {
                return true;
            }
        }();

        struct cursor_type {
            cursor_t<Base> outer_cur;
            std::variant<cursor_t<Pattern>, cursor_t<InnerSeq>> inner_cur{};

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool = default;
        };

        static constexpr auto satisfy(auto& self, cursor_type& cur) -> void
        {
            while (true) {
                if (cur.inner_cur.index() == 0) {
                    if (!flux::is_last(self.pattern_, std::get<0>(cur.inner_cur))) {
                        break;
                    }

                    auto& inner = flux::read_at(self.base_, cur.outer_cur);
                    variant_emplace<1>(cur.inner_cur, flux::first(inner));
                } else {
                    auto& inner = flux::read_at(self.base_, cur.outer_cur);
                    if (!flux::is_last(inner, std::get<1>(cur.inner_cur))) {
                        break;
                    }

                    flux::inc(self.base_, cur.outer_cur);
                    variant_emplace<0>(cur.inner_cur, flux::first(self.pattern_));
                    if (flux::is_last(self.base_, cur.outer_cur)) {
                        break;
                    }
                }
            }
        }

    public:
        using value_type = std::common_type_t<value_t<InnerSeq>, value_t<Pattern>>;

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto first(Self& self) -> cursor_type
        {
            cursor_type cur{.outer_cur = flux::first(self.base_)};
            if (!flux::is_last(self.base_, cur.outer_cur)) {
                auto& inner = flux::read_at(self.base_, cur.outer_cur);
                variant_emplace<1>(cur.inner_cur, flux::first(inner));
            }
            satisfy(self, cur);
            return cur;
        }

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.outer_cur);
        }

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur)
            -> decltype(auto)
        {
            using R = std::common_reference_t<
                element_t<element_t<decltype((self.base_))>>,
                element_t<decltype((self.pattern_))>>;

            if (cur.inner_cur.index() == 0) {
                return static_cast<R>(flux::read_at(self.pattern_, std::get<0>(cur.inner_cur)));
            } else {
                auto& inner = flux::read_at(self.base_, cur.outer_cur);
                return static_cast<R>(flux::read_at(inner, std::get<1>(cur.inner_cur)));
            }
        }

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto move_at(Self& self, cursor_type const& cur)
            -> decltype(auto)
        {
            using R = std::common_reference_t<
                      rvalue_element_t<element_t<decltype((self.base_))>>,
                      rvalue_element_t<decltype((self.pattern_))>>;

            if (cur.inner_cur.index() == 0) {
                return static_cast<R>(flux::move_at(self.pattern_, std::get<0>(cur.inner_cur)));
            } else {
                auto& inner = flux::read_at(self.base_, cur.outer_cur);
                return static_cast<R>(flux::move_at(inner, std::get<1>(cur.inner_cur)));
            }
        }


        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto inc(Self& self, cursor_type& cur) -> void
        {
            if (cur.inner_cur.index() == 0) {
                flux::inc(self.pattern_, std::get<0>(cur.inner_cur));
            } else {
                auto& inner = flux::read_at(self.base_, cur.outer_cur);
                flux::inc(inner, std::get<1>(cur.inner_cur));
            }
            satisfy(self, cur);
        }

        template <typename Self>
            requires can_flatten<Self> && bounded_sequence<Base>
        static constexpr auto last(Self& self) -> cursor_type
        {
            return cursor_type{.outer_cur = flux::last(self.base_)};
        }

        template <typename Self>
            requires can_flatten<Self> &&
                     bidirectional_sequence<Base> &&
                     bidirectional_sequence<InnerSeq> &&
                     bounded_sequence<InnerSeq> &&
                     bidirectional_sequence<Pattern> &&
                     bounded_sequence<Pattern>
        static constexpr auto dec(Self& self, cursor_type& cur) -> void
        {
            if (flux::is_last(self.base_, cur.outer_cur)) {
                flux::dec(self.base_, cur.outer_cur);
                auto& inner = flux::read_at(self.base_, cur.outer_cur);
                variant_emplace<1>(cur.inner_cur, flux::last(inner));
            }

            while (true) {
                if (cur.inner_cur.index() == 0) {
                    if (std::get<0>(cur.inner_cur) == flux::first(self.pattern_)) {
                        flux::dec(self.base_, cur.outer_cur);
                        auto& inner = flux::read_at(self.base_, cur.outer_cur);
                        variant_emplace<1>(cur.inner_cur, flux::last(inner));
                    } else {
                        break;
                    }
                } else {
                    auto& inner = flux::read_at(self.base_, cur.outer_cur);
                    if (std::get<1>(cur.inner_cur) == flux::first(inner)) {
                        variant_emplace<0>(cur.inner_cur, flux::last(self.pattern_));
                    } else {
                        break;
                    }
                }
            }

            if (cur.inner_cur.index() == 0) {
                flux::dec(self.pattern_, std::get<0>(cur.inner_cur));
            } else {
                auto& inner = flux::read_at(self.base_, cur.outer_cur);
                flux::dec(inner, std::get<1>(cur.inner_cur));
            }
        }
    };
};

struct flatten_with_fn {

    template <adaptable_sequence Seq, adaptable_sequence Pattern>
        requires sequence<element_t<Seq>> &&
                 multipass_sequence<Pattern> &&
                 flatten_with_compatible<element_t<Seq>, Pattern>
    constexpr auto operator()(Seq&& seq, Pattern&& pattern) const
        -> sequence auto
    {
        return flatten_with_adaptor<std::decay_t<Seq>, std::decay_t<Pattern>>(
            FLUX_FWD(seq), FLUX_FWD(pattern));
    }

    template <adaptable_sequence Seq>
        requires sequence<element_t<Seq>> &&
                 std::movable<value_t<element_t<Seq>>>
    constexpr auto operator()(Seq&& seq, value_t<element_t<Seq>> value) const
        -> sequence auto
    {
        return (*this)(FLUX_FWD(seq), flux::single(std::move(value)));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto flatten_with = detail::flatten_with_fn{};

template <typename Derived>
template <adaptable_sequence Pattern>
    requires sequence<element_t<Derived>> &&
             multipass_sequence<Pattern> &&
             detail::flatten_with_compatible<element_t<Derived>, Pattern>
constexpr auto inline_sequence_base<Derived>::flatten_with(Pattern&& pattern) &&
{
    return flux::flatten_with(std::move(derived()), FLUX_FWD(pattern));
}

template <typename Derived>
template <typename Value>
    requires sequence<element_t<Derived>> &&
             std::constructible_from<value_t<element_t<Derived>>, Value&&>
constexpr auto inline_sequence_base<Derived>::flatten_with(Value value) &&
{
    return flux::flatten_with(std::move(derived()), std::move(value));
}

} // namespace flux

#endif // FLUX_OP_FLATTEN_WITH_HPP_INCLUDED





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

FLUX_EXPORT inline constexpr auto inplace_reverse = detail::inplace_reverse_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::inplace_reverse()
    requires bounded_sequence<D> && detail::element_swappable_with<D, D>
{
    return flux::inplace_reverse(derived());
}

} // namespace flux

#endif // FLUX_OP_INPLACE_REVERSE_HPP_INCLUDED



// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_MASK_HPP_INCLUDED
#define FLUX_OP_MASK_HPP_INCLUDED



namespace flux {

namespace detail {

template <sequence Base, sequence Mask>
struct mask_adaptor : inline_sequence_base<mask_adaptor<Base, Mask>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Mask mask_;

public:
    constexpr mask_adaptor(decays_to<Base> auto&& base, decays_to<Mask> auto&& mask)
        : base_(FLUX_FWD(base)),
          mask_(FLUX_FWD(mask))
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;
            cursor_t<Mask> mask_cur;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool
                requires std::equality_comparable<cursor_t<Base>> &&
                         std::equality_comparable<cursor_t<Mask>>
                = default;
        };

        template <typename Self>
        static inline constexpr bool maybe_const_iterable =
            std::is_const_v<Self>
                ? sequence<Base const> && sequence<Mask const>
                : true;

    public:
        using value_type = value_t<Base>;

        static inline constexpr bool is_infinite =
            infinite_sequence<Base> && infinite_sequence<Mask>;

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto first(Self& self) -> cursor_type
        {
            auto base_cur = flux::first(self.base_);
            auto mask_cur = flux::first(self.mask_);

            while (!flux::is_last(self.base_, base_cur) && !flux::is_last(self.mask_, mask_cur)) {
                if (static_cast<bool>(flux::read_at(self.mask_, mask_cur))) {
                    break;
                }
                flux::inc(self.base_, base_cur);
                flux::inc(self.mask_, mask_cur);
            }
            return cursor_type{std::move(base_cur), std::move(mask_cur)};
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.base_cur) ||
                    flux::is_last(self.mask_, cur.mask_cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto inc(Self& self, cursor_type& cur) -> void
        {
            while (!flux::is_last(self.base_, flux::inc(self.base_, cur.base_cur)) &&
                   !flux::is_last(self.mask_, flux::inc(self.mask_, cur.mask_cur))) {
                if (static_cast<bool>(flux::read_at(self.mask_, cur.mask_cur))) {
                    break;
                }
            }
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur) -> decltype(auto)
        {
            return flux::read_at(self.base_, cur.base_cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto read_at_unchecked(Self& self, cursor_type const& cur)
            -> decltype(auto)
        {
            return flux::read_at_unchecked(self.base_, cur.base_cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto move_at(Self& self, cursor_type const& cur) -> decltype(auto)
        {
            return flux::move_at(self.base_, cur.base_cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto move_at_unchecked(Self& self, cursor_type const& cur)
            -> decltype(auto)
        {
            return flux::move_at_unchecked(self.base_, cur.base_cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self> &&
                     bounded_sequence<Base> &&
                     bounded_sequence<Mask>
        static constexpr auto last(Self& self) -> cursor_type
        {
            return cursor_type{.base_cur = flux::last(self.base_),
                               .mask_cur = flux::last(self.mask_)};
        }

        template <typename Self>
            requires maybe_const_iterable<Self> &&
                     bidirectional_sequence<Base> &&
                     bidirectional_sequence<Mask>
        static constexpr auto dec(Self& self, cursor_type& cur) -> void
        {
            do {
                flux::dec(self.base_, cur.base_cur);
                flux::dec(self.mask_, cur.mask_cur);
            } while (!static_cast<bool>(flux::read_at(self.mask_, cur.mask_cur)));
        }
    };
};

struct mask_fn {
    template <adaptable_sequence Base, adaptable_sequence Mask>
        requires boolean_testable<element_t<Mask>>
    [[nodiscard]]
    constexpr auto operator()(Base&& base, Mask&& mask) const
    {
        return mask_adaptor<std::decay_t<Base>, std::decay_t<Mask>>(
            FLUX_FWD(base), FLUX_FWD(mask));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto mask = detail::mask_fn{};

template <typename D>
template <adaptable_sequence Mask>
    requires detail::boolean_testable<element_t<Mask>>
constexpr auto inline_sequence_base<D>::mask(Mask&& mask_) &&
{
    return flux::mask(std::move(derived()), FLUX_FWD(mask_));
}

} // namespace flux

#endif // FLUX_OP_MASK_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_READ_ONLY_HPP_INCLUDED
#define FLUX_OP_READ_ONLY_HPP_INCLUDED




namespace flux {

namespace detail {

template <typename T>
struct cast_to_const {
    constexpr auto operator()(auto&& elem) const -> T { return FLUX_FWD(elem); }
};

template <sequence Base>
    requires (not read_only_sequence<Base>)
struct read_only_adaptor : map_adaptor<Base, cast_to_const<const_element_t<Base>>> {
private:
    using map = map_adaptor<Base, cast_to_const<const_element_t<Base>>>;

public:
    constexpr explicit read_only_adaptor(decays_to<Base> auto&& base)
        : map(FLUX_FWD(base), cast_to_const<const_element_t<Base>>{})
    {}

    struct flux_sequence_traits : map::flux_sequence_traits {
    private:
        using const_rvalue_element_t = std::common_reference_t<
            value_t<Base> const&&, rvalue_element_t<Base>>;

    public:
        using value_type = value_t<Base>;

        static constexpr auto move_at(auto& self, cursor_t<Base> const& cur)
            -> const_rvalue_element_t
        {
            return static_cast<const_rvalue_element_t>(flux::move_at(self.base(), cur));
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_t<Base> const& cur)
            -> const_rvalue_element_t
        {
            return static_cast<const_rvalue_element_t>(flux::move_at_unchecked(self.base(), cur));
        }

        static constexpr auto data(auto& self)
            requires contiguous_sequence<Base>
        {
            using P = std::add_pointer_t<std::remove_reference_t<const_element_t<Base>>>;
            return static_cast<P>(flux::data(self.base()));
        }
    };
};

struct read_only_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> read_only_sequence auto
    {
        if constexpr (read_only_sequence<Seq>) {
            return FLUX_FWD(seq);
        } else {
            return read_only_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
        }
    }
};


} // namespace detail

FLUX_EXPORT inline constexpr auto read_only = detail::read_only_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::read_only() &&
{
    return flux::read_only(std::move(derived()));
}

} // namespace flux

#endif




// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SCAN_HPP_INCLUDED
#define FLUX_OP_SCAN_HPP_INCLUDED




#include <utility> // for std::as_const

namespace flux {

namespace detail {

enum class scan_mode {
    inclusive,
    exclusive
};

template <scan_mode>
struct scan_cursor_base {};

template <>
struct scan_cursor_base<scan_mode::exclusive> {
    bool is_last;
};

template <typename Base, typename Func, typename R, scan_mode Mode>
struct scan_adaptor : inline_sequence_base<scan_adaptor<Base, Func, R, Mode>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;
    FLUX_NO_UNIQUE_ADDRESS R accum_;

public:
    constexpr scan_adaptor(decays_to<Base> auto&& base, Func&& func, auto&& init)
        : base_(FLUX_FWD(base)),
          func_(std::move(func)),
          accum_(FLUX_FWD(init))
    {}

    scan_adaptor(scan_adaptor&&) = default;
    scan_adaptor& operator=(scan_adaptor&&) = default;

    struct flux_sequence_traits : default_sequence_traits {
    private:

        struct cursor_type : private scan_cursor_base<Mode> {
            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;

        private:
            friend struct flux_sequence_traits;

            constexpr explicit cursor_type(cursor_t<Base>&& base_cur)
                : base_cur(std::move(base_cur))
            {}

            cursor_t<Base> base_cur;
        };

        using self_t = scan_adaptor;

        static constexpr auto update(self_t& self, cursor_t<Base> const& cur) -> void
        {
            if (!flux::is_last(self.base_, cur)) {
                self.accum_ = std::invoke(self.func_, std::move(self.accum_),
                                          flux::read_at(self.base_, cur));
            }
        }

    public:
        static inline constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto first(self_t& self) -> cursor_type
        {
            auto cur = flux::first(self.base_);
            if constexpr (Mode == scan_mode::inclusive) {
                update(self, cur);
                return cursor_type(std::move(cur));
            } else if constexpr (Mode == scan_mode::exclusive) {
                bool last = flux::is_last(self.base_, cur);
                cursor_type out = cursor_type(std::move(cur));
                out.is_last = last;
                return out;
            }
        }

        static constexpr auto is_last(self_t& self, cursor_type const& cur) -> bool
        {
            if constexpr (Mode == scan_mode::exclusive) {
                return cur.is_last;
            } else {
                return flux::is_last(self.base_, cur.base_cur);
            }
        }

        static constexpr auto inc(self_t& self, cursor_type& cur) -> void
        {
            if constexpr (Mode == scan_mode::inclusive) {
                flux::inc(self.base_, cur.base_cur);
                update(self, cur.base_cur);
            } else {
                update(self, cur.base_cur);
                if (flux::is_last(self.base_, cur.base_cur)) {
                    cur.is_last = true;
                } else {
                    flux::inc(self.base_, cur.base_cur);
                }
            }
        }

        static constexpr auto read_at(self_t& self, cursor_type const&) -> R const&
        {
            return self.accum_;
        }

        static constexpr auto last(self_t& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            auto cur = cursor_type(flux::last(self.base_));
            if constexpr (Mode == scan_mode::exclusive) {
                cur.is_last = true;
            }
            return cur;
        }

        static constexpr auto size(self_t& self) -> distance_t
            requires sized_sequence<Base>
        {
            if constexpr (Mode == scan_mode::exclusive) {
                return num::checked_add(flux::size(self.base_), distance_t{1});
            } else {
                return flux::size(self.base_);
            }
        }

        static constexpr auto for_each_while(self_t& self, auto&& pred) -> cursor_type
            requires (Mode != scan_mode::exclusive)
        {
            return cursor_type(flux::for_each_while(self.base_, [&](auto&& elem) {
                self.accum_ = std::invoke(self.func_, std::move(self.accum_), FLUX_FWD(elem));
                return std::invoke(pred, std::as_const(self.accum_));
            }));
        }

        using default_sequence_traits::for_each_while; // when Mode == exclusive
    };
};

struct scan_fn {
    template <adaptable_sequence Seq, typename Func, std::movable Init = value_t<Seq>,
              typename R = fold_result_t<Seq, Func, Init>>
        requires foldable<Seq, Func, Init>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func, Init init = Init{}) const
        -> sequence auto
    {
        return scan_adaptor<std::decay_t<Seq>, Func, R, scan_mode::inclusive>(
            FLUX_FWD(seq), std::move(func), std::move(init));
    }
};

struct prescan_fn {
    template <adaptable_sequence Seq, typename Func, std::movable Init,
              typename R = fold_result_t<Seq, Func, Init>>
        requires foldable<Seq, Func, Init>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func, Init init) const
        -> sequence auto
    {
        return scan_adaptor<std::decay_t<Seq>, Func, R, scan_mode::exclusive>(
            FLUX_FWD(seq), std::move(func), std::move(init));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto scan = detail::scan_fn{};
FLUX_EXPORT inline constexpr auto prescan = detail::prescan_fn{};

template <typename Derived>
template <typename D, typename Func, typename Init>
    requires foldable<Derived, Func, Init>
constexpr auto inline_sequence_base<Derived>::scan(Func func, Init init) &&
{
    return flux::scan(std::move(derived()), std::move(func), std::move(init));
}

template <typename Derived>
template <typename Func, typename Init>
    requires foldable<Derived, Func, Init>
constexpr auto inline_sequence_base<Derived>::prescan(Func func, Init init) &&
{
    return flux::prescan(std::move(derived()), std::move(func), std::move(init));
}

} // namespace flux

#endif // FLUX_OP_SCAN_HPP_INCLUDED


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SCAN_FIRST_HPP_INCLUDED
#define FLUX_OP_SCAN_FIRST_HPP_INCLUDED




#include <utility> // for std::as_const

namespace flux {

namespace detail {

template <typename Base, typename Func, typename R>
struct scan_first_adaptor : inline_sequence_base<scan_first_adaptor<Base, Func, R>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Func func_;
    flux::optional<R> accum_;

public:
    constexpr scan_first_adaptor(decays_to<Base> auto&& base, Func&& func)
        : base_(FLUX_FWD(base)),
          func_(std::move(func))
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;

        private:
            friend struct flux_sequence_traits;

            constexpr explicit cursor_type(cursor_t<Base>&& base_cur)
                : base_cur(std::move(base_cur))
            {}

            cursor_t<Base> base_cur;
        };

        using self_t = scan_first_adaptor;

    public:
        static constexpr auto first(self_t& self) -> cursor_type
        {
            auto cur = flux::first(self.base_);
            if (!flux::is_last(self.base_, cur)) {
                self.accum_.emplace(flux::read_at(self.base_, cur));
            }
            return cursor_type(std::move(cur));
        }

        static constexpr auto is_last(self_t& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.base_cur);
        }

        static constexpr auto inc(self_t& self, cursor_type& cur) -> void
        {
            flux::inc(self.base_, cur.base_cur);
            if (!flux::is_last(self.base_, cur.base_cur)) {
                self.accum_.emplace(
                    std::invoke(self.func_,
                                std::move(self.accum_.value_unchecked()),
                                flux::read_at(self.base_, cur.base_cur)));
            }
        }

        static constexpr auto read_at(self_t& self, cursor_type const&) -> R const&
        {
            return self.accum_.value();
        }

        static constexpr auto read_at_unchecked(self_t& self, cursor_type const&)
            -> R const&
        {
            return self.accum_.value_unchecked();
        }

        static constexpr auto last(self_t& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            return cursor_type(flux::last(self.base_));
        }

        static constexpr auto size(self_t& self) -> distance_t
            requires sized_sequence<Base>
        {
            return flux::size(self.base_);
        }

        static constexpr auto for_each_while(self_t& self, auto&& pred) -> cursor_type
        {
            return cursor_type(flux::for_each_while(self.base_, [&](auto&& elem) {
                if (self.accum_.has_value()) {
                    self.accum_.emplace(
                        std::invoke(self.func_,
                                    std::move(self.accum_.value_unchecked()),
                                    FLUX_FWD(elem)));
                } else {
                    self.accum_.emplace(FLUX_FWD(elem));
                }
                return std::invoke(pred, self.accum_.value_unchecked());
            }));
        }
    };
};

struct scan_first_fn {
    template <adaptable_sequence Seq, typename Func>
        requires foldable<Seq, Func, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Func func) const -> sequence auto
    {
        using R = fold_result_t<Seq, Func, element_t<Seq>>;
        return scan_first_adaptor<std::decay_t<Seq>, Func, R>(
            FLUX_FWD(seq), std::move(func));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto scan_first = detail::scan_first_fn{};

template <typename Derived>
template <typename Func>
    requires foldable<Derived, Func, element_t<Derived>>
constexpr auto inline_sequence_base<Derived>::scan_first(Func func) &&
{
    return flux::scan_first(std::move(derived()), std::move(func));
}

} // namespace flux

#endif // FLUX_OP_SCAN_FIRST_HPP_INCLUDED


// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SET_ADAPTORS_HPP_INCLUDED
#define FLUX_OP_SET_ADAPTORS_HPP_INCLUDED




#include <utility>

namespace flux::detail {

template <sequence Base1, sequence Base2, typename Cmp>
struct set_union_adaptor
    : flux::inline_sequence_base<set_union_adaptor<Base1, Base2, Cmp>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base1 base1_;
    FLUX_NO_UNIQUE_ADDRESS Base2 base2_;
    FLUX_NO_UNIQUE_ADDRESS Cmp cmp_;

public:
    constexpr set_union_adaptor(decays_to<Base1> auto&& base1, decays_to<Base2> auto&& base2, Cmp cmp)
        : base1_(FLUX_FWD(base1)),
          base2_(FLUX_FWD(base2)),
          cmp_(cmp)
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:

        struct cursor_type {
            cursor_t<Base1> base1_cursor;
            cursor_t<Base2> base2_cursor;
            enum : bool {first, second} active_ = first;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool
                requires std::equality_comparable<cursor_t<Base1>> &&
                         std::equality_comparable<cursor_t<Base2>>
                = default;
        };

        template <typename Self>
        static inline constexpr bool maybe_const_iterable
            = std::is_const_v<Self> ? (flux::sequence<Base1 const> && flux::sequence<Base2 const>) : true;

        template <typename Self>
        static constexpr void update(Self& self, cursor_type& cur) {
            if (flux::is_last(self.base1_, cur.base1_cursor)) {
                cur.active_ = cursor_type::second;
                return;
            }

            if (flux::is_last(self.base2_, cur.base2_cursor)) {
                cur.active_ = cursor_type::first;
                return;
            }

            auto r = std::invoke(self.cmp_, flux::read_at(self.base1_, cur.base1_cursor),
                                            flux::read_at(self.base2_, cur.base2_cursor));

            if (r == std::weak_ordering::greater) {
                cur.active_ = cursor_type::second;
                return;
            } else if (r == std::weak_ordering::equivalent) {
                flux::inc(self.base2_, cur.base2_cursor);
            }

            cur.active_ = cursor_type::first;
        }

    public:
        using value_type = std::common_type_t<value_t<Base1>, value_t<Base2>>;

        inline static constexpr bool is_infinite = flux::infinite_sequence<Base1> || 
                                                   flux::infinite_sequence<Base2>;

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto first(Self& self) -> cursor_type
        {
            auto cur = cursor_type{.base1_cursor = flux::first(self.base1_),
                                   .base2_cursor = flux::first(self.base2_)};
            update(self, cur);
            return cur;
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base1_, cur.base1_cursor) && 
                   flux::is_last(self.base2_, cur.base2_cursor);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto inc(Self& self, cursor_type& cur) -> void
        {
            if (cur.active_ == cursor_type::first) {
                flux::inc(self.base1_, cur.base1_cursor);
            } else {
                flux::inc(self.base2_, cur.base2_cursor);
            }

            update(self, cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur)
            -> std::common_reference_t<decltype(flux::read_at(self.base1_, cur.base1_cursor)), 
                                       decltype(flux::read_at(self.base2_, cur.base2_cursor))>
        {
            if (cur.active_ == cursor_type::first) {
                return flux::read_at(self.base1_, cur.base1_cursor);
            } else {
                return flux::read_at(self.base2_, cur.base2_cursor);
            }
        }

        template <typename Self>
            requires maybe_const_iterable<Self> && 
                     bounded_sequence<Base1> && bounded_sequence<Base2>
        static constexpr auto last(Self& self) -> cursor_type
        {
            return cursor_type{flux::last(self.base1_), flux::last(self.base2_), cursor_type::second};
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto move_at(Self& self, cursor_type const& cur)
            -> std::common_reference_t<decltype(flux::move_at(self.base1_, cur.base1_cursor)), 
                                       decltype(flux::move_at(self.base2_, cur.base2_cursor))>
        {
            if (cur.active_ == cursor_type::first) {
                return flux::move_at(self.base1_, cur.base1_cursor);
            } else {
                return flux::move_at(self.base2_, cur.base2_cursor);
            }
        }
        
    };
};

template <sequence Base1, sequence Base2, typename Cmp>
struct set_difference_adaptor
    : flux::inline_sequence_base<set_difference_adaptor<Base1, Base2, Cmp>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base1 base1_;
    FLUX_NO_UNIQUE_ADDRESS Base2 base2_;
    FLUX_NO_UNIQUE_ADDRESS Cmp cmp_;

public:
    constexpr set_difference_adaptor(decays_to<Base1> auto&& base1, decays_to<Base2> auto&& base2, Cmp cmp)
        : base1_(FLUX_FWD(base1)),
          base2_(FLUX_FWD(base2)),
          cmp_(cmp)
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:

        struct cursor_type {
            cursor_t<Base1> base1_cursor;
            cursor_t<Base2> base2_cursor;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool
                requires std::equality_comparable<cursor_t<Base1>> &&
                         std::equality_comparable<cursor_t<Base2>>
                = default;
        };

        template <typename Self>
        static inline constexpr bool maybe_const_iterable
            = std::is_const_v<Self> ? (flux::sequence<Base1 const> && flux::sequence<Base2 const>) : true;

        template <typename Self>
        static constexpr void update(Self& self, cursor_type& cur) {
            while(not flux::is_last(self.base1_, cur.base1_cursor))
            {
                if(flux::is_last(self.base2_, cur.base2_cursor)) {
                    return;
                }

                auto r = std::invoke(self.cmp_, flux::read_at(self.base1_, cur.base1_cursor),
                                                flux::read_at(self.base2_, cur.base2_cursor));

                if (r == std::weak_ordering::less) {
                    return;
                } else if (r == std::weak_ordering::equivalent) {
                    flux::inc(self.base1_, cur.base1_cursor);
                }

                flux::inc(self.base2_, cur.base2_cursor);
            }
        }

    public:
        using value_type = value_t<Base1>;

        inline static constexpr bool is_infinite = flux::infinite_sequence<Base1>;

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto first(Self& self) -> cursor_type
        {
            auto cur = cursor_type{.base1_cursor = flux::first(self.base1_),
                                   .base2_cursor = flux::first(self.base2_)};
            update(self, cur);
            return cur;
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base1_, cur.base1_cursor);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto inc(Self& self, cursor_type& cur) -> void
        {
            flux::inc(self.base1_, cur.base1_cursor);
            update(self, cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur)
            -> decltype(flux::read_at(self.base1_, cur.base1_cursor))
        {
            return flux::read_at(self.base1_, cur.base1_cursor);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto move_at(Self& self, cursor_type const& cur)
            -> decltype(flux::move_at(self.base1_, cur.base1_cursor))
        {
            return flux::move_at(self.base1_, cur.base1_cursor);
        }
        
    };
};

template <sequence Base1, sequence Base2, typename Cmp>
struct set_symmetric_difference_adaptor
    : flux::inline_sequence_base<set_symmetric_difference_adaptor<Base1, Base2, Cmp>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base1 base1_;
    FLUX_NO_UNIQUE_ADDRESS Base2 base2_;
    FLUX_NO_UNIQUE_ADDRESS Cmp cmp_;

public:
    constexpr set_symmetric_difference_adaptor(decays_to<Base1> auto&& base1, decays_to<Base2> auto&& base2, Cmp cmp)
        : base1_(FLUX_FWD(base1)),
          base2_(FLUX_FWD(base2)),
          cmp_(cmp)
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:

        struct cursor_type {
            cursor_t<Base1> base1_cursor;
            cursor_t<Base2> base2_cursor;
            enum : char {first, second, first_done, second_done} state_ = first;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool
                requires std::equality_comparable<cursor_t<Base1>> &&
                         std::equality_comparable<cursor_t<Base2>>
                = default;
        };

        template <typename Self>
        static inline constexpr bool maybe_const_iterable
            = std::is_const_v<Self> ? (flux::sequence<Base1 const> && flux::sequence<Base2 const>) : true;

        template <typename Self>
        static constexpr void update(Self& self, cursor_type& cur) {
            while(not flux::is_last(self.base1_, cur.base1_cursor))
            {
                if(flux::is_last(self.base2_, cur.base2_cursor)) {
                    cur.state_ = cursor_type::second_done;
                    return;
                }

                auto r = std::invoke(self.cmp_, flux::read_at(self.base1_, cur.base1_cursor),
                                                flux::read_at(self.base2_, cur.base2_cursor));

                if (r == std::weak_ordering::less) {
                    cur.state_ = cursor_type::first;
                    return;
                } else if (r == std::weak_ordering::greater) {
                    cur.state_ = cursor_type::second;
                    return;
                } else {
                    flux::inc(self.base1_, cur.base1_cursor);
                    flux::inc(self.base2_, cur.base2_cursor);
                }
            }
            cur.state_ = cursor_type::first_done;
        }

    public:
        using value_type = std::common_type_t<value_t<Base1>, value_t<Base2>>;

        inline static constexpr bool is_infinite = flux::infinite_sequence<Base1> || 
                                                   flux::infinite_sequence<Base2>;

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto first(Self& self) -> cursor_type
        {
            auto cur = cursor_type{.base1_cursor = flux::first(self.base1_),
                                   .base2_cursor = flux::first(self.base2_)};
            update(self, cur);
            return cur;
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base1_, cur.base1_cursor) && 
                   flux::is_last(self.base2_, cur.base2_cursor);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto inc(Self& self, cursor_type& cur) -> void
        {
            switch (cur.state_) {
                case cursor_type::first:
                    flux::inc(self.base1_, cur.base1_cursor);
                    break;
                case cursor_type::second:
                    flux::inc(self.base2_, cur.base2_cursor);
                    break;
                case cursor_type::first_done:
                    flux::inc(self.base2_, cur.base2_cursor);
                    return;
                case cursor_type::second_done:
                    flux::inc(self.base1_, cur.base1_cursor);
                    return;
            }

            update(self, cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur)
            -> std::common_reference_t<decltype(flux::read_at(self.base1_, cur.base1_cursor)),
                                       decltype(flux::read_at(self.base2_, cur.base2_cursor))>
        {
            using R = std::common_reference_t<decltype(flux::read_at(self.base1_, cur.base1_cursor)),
                                              decltype(flux::read_at(self.base2_, cur.base2_cursor))>;
            if (cur.state_ == cursor_type::first || cur.state_ == cursor_type::second_done) {
                return static_cast<R>(flux::read_at(self.base1_, cur.base1_cursor));
            } else {
                return static_cast<R>(flux::read_at(self.base2_, cur.base2_cursor));
            }
        }

        template <typename Self>
            requires maybe_const_iterable<Self> && bounded_sequence<Base1>
        static constexpr auto last(Self& self) -> cursor_type
        {
            return cursor_type{flux::last(self.base1_), flux::last(self.base2_)};
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto move_at(Self& self, cursor_type const& cur)
            -> std::common_reference_t<decltype(flux::move_at(self.base1_, cur.base1_cursor)),
                                       decltype(flux::move_at(self.base2_, cur.base2_cursor))>
        {
            using R = std::common_reference_t<decltype(flux::move_at(self.base1_, cur.base1_cursor)),
                                              decltype(flux::move_at(self.base2_, cur.base2_cursor))>;
            if (cur.state_ == cursor_type::first || cur.state_ == cursor_type::second_done) {
                return static_cast<R>(flux::move_at(self.base1_, cur.base1_cursor));
            } else {
                return static_cast<R>(flux::move_at(self.base2_, cur.base2_cursor));
            }
        }
    };
};

template <sequence Base1, sequence Base2, typename Cmp>
struct set_intersection_adaptor
    : flux::inline_sequence_base<set_intersection_adaptor<Base1, Base2, Cmp>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base1 base1_;
    FLUX_NO_UNIQUE_ADDRESS Base2 base2_;
    FLUX_NO_UNIQUE_ADDRESS Cmp cmp_;

public:
    constexpr set_intersection_adaptor(decays_to<Base1> auto&& base1, decays_to<Base2> auto&& base2, Cmp cmp)
        : base1_(FLUX_FWD(base1)),
          base2_(FLUX_FWD(base2)),
          cmp_(cmp)
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:

        struct cursor_type {
            cursor_t<Base1> base1_cursor;
            cursor_t<Base2> base2_cursor;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool
                requires std::equality_comparable<cursor_t<Base1>> &&
                         std::equality_comparable<cursor_t<Base2>>
                = default;
        };

        template <typename Self>
        static inline constexpr bool maybe_const_iterable
            = std::is_const_v<Self> ? (flux::sequence<Base1 const> && flux::sequence<Base2 const>) : true;

        template <typename Self>
        static constexpr void update(Self& self, cursor_type& cur) {
            while(not flux::is_last(self.base1_, cur.base1_cursor) && 
                  not flux::is_last(self.base2_, cur.base2_cursor))
            {
                auto r = std::invoke(self.cmp_, flux::read_at(self.base1_, cur.base1_cursor),
                                                flux::read_at(self.base2_, cur.base2_cursor));

                if (r == std::weak_ordering::less) {
                    flux::inc(self.base1_, cur.base1_cursor);
                } else if (r == std::weak_ordering::greater) {
                    flux::inc(self.base2_, cur.base2_cursor);
                } else {
                    return;
                }
            }
        }

    public:
        using value_type = value_t<Base1>;

        inline static constexpr bool is_infinite = flux::infinite_sequence<Base1>;

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto first(Self& self) -> cursor_type
        {
            auto cur = cursor_type{.base1_cursor = flux::first(self.base1_),
                                   .base2_cursor = flux::first(self.base2_)};
            update(self, cur);
            return cur;
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base1_, cur.base1_cursor) || 
                   flux::is_last(self.base2_, cur.base2_cursor);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto inc(Self& self, cursor_type& cur) -> void
        {
            flux::inc(self.base1_, cur.base1_cursor);
            flux::inc(self.base2_, cur.base2_cursor);
            update(self, cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur)
            -> decltype(flux::read_at(self.base1_, cur.base1_cursor))
        {
            return flux::read_at(self.base1_, cur.base1_cursor);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto move_at(Self& self, cursor_type const& cur)
            -> decltype(flux::move_at(self.base1_, cur.base1_cursor))
        {
            return flux::move_at(self.base1_, cur.base1_cursor);
        }
        
    };
};

template <typename T1, typename T2>
concept set_op_compatible =
    std::common_reference_with<element_t<T1>, element_t<T2>> &&
    std::common_reference_with<rvalue_element_t<T1>, rvalue_element_t<T2>> &&
    requires { typename std::common_type_t<value_t<T1>, value_t<T2>>; };

struct set_union_fn {
    template <adaptable_sequence Seq1, adaptable_sequence Seq2, typename Cmp = std::compare_three_way>
        requires set_op_compatible<Seq1, Seq2> && 
                 weak_ordering_for<Cmp, Seq1> &&
                 weak_ordering_for<Cmp, Seq2>
    [[nodiscard]]
    constexpr auto operator()(Seq1&& seq1, Seq2&& seq2, Cmp cmp = {}) const
    {
        return set_union_adaptor<std::decay_t<Seq1>, std::decay_t<Seq2>, Cmp>(FLUX_FWD(seq1), FLUX_FWD(seq2), cmp);
    }
};

struct set_difference_fn {
    template <adaptable_sequence Seq1, adaptable_sequence Seq2, typename Cmp = std::compare_three_way>
        requires weak_ordering_for<Cmp, Seq1> &&
                 weak_ordering_for<Cmp, Seq2>
    [[nodiscard]]
    constexpr auto operator()(Seq1&& seq1, Seq2&& seq2, Cmp cmp = {}) const
    {
        return set_difference_adaptor<std::decay_t<Seq1>, std::decay_t<Seq2>, Cmp>(FLUX_FWD(seq1), FLUX_FWD(seq2), cmp);
    }
};

struct set_symmetric_difference_fn {
    template <adaptable_sequence Seq1, adaptable_sequence Seq2, typename Cmp = std::compare_three_way>
        requires set_op_compatible<Seq1, Seq2> &&
                 weak_ordering_for<Cmp, Seq1> &&
                 weak_ordering_for<Cmp, Seq2>
    [[nodiscard]]
    constexpr auto operator()(Seq1&& seq1, Seq2&& seq2, Cmp cmp = {}) const
    {
        return set_symmetric_difference_adaptor<std::decay_t<Seq1>, std::decay_t<Seq2>, Cmp>(FLUX_FWD(seq1), FLUX_FWD(seq2), cmp);
    }
};

struct set_intersection_fn {
    template <adaptable_sequence Seq1, adaptable_sequence Seq2, typename Cmp = std::compare_three_way>
        requires weak_ordering_for<Cmp, Seq1> &&
                 weak_ordering_for<Cmp, Seq2>
    [[nodiscard]]
    constexpr auto operator()(Seq1&& seq1, Seq2&& seq2, Cmp cmp = {}) const
    {
        return set_intersection_adaptor<std::decay_t<Seq1>, std::decay_t<Seq2>, Cmp>(FLUX_FWD(seq1), FLUX_FWD(seq2), cmp);
    }
};

} // namespace detail

namespace flux {

FLUX_EXPORT inline constexpr auto set_union = detail::set_union_fn{};
FLUX_EXPORT inline constexpr auto set_difference = detail::set_difference_fn{};
FLUX_EXPORT inline constexpr auto set_symmetric_difference = detail::set_symmetric_difference_fn{};
FLUX_EXPORT inline constexpr auto set_intersection = detail::set_intersection_fn{};

} // namespace flux

#endif // namespace FLUX_OP_SET_ADAPTORS_HPP_INCLUDED



// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SLIDE_HPP_INCLUDED
#define FLUX_OP_SLIDE_HPP_INCLUDED





namespace flux {

namespace detail {

template <multipass_sequence Base>
struct slide_adaptor : inline_sequence_base<slide_adaptor<Base>> {
private:
    Base base_;
    distance_t win_sz_;

public:
    constexpr slide_adaptor(decays_to<Base> auto&& base, distance_t win_sz)
        : base_(FLUX_FWD(base)),
          win_sz_(win_sz)
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> from;
            cursor_t<Base> to;

            friend constexpr auto operator==(cursor_type const& lhs, cursor_type const& rhs)
                -> bool
            {
                return lhs.from == rhs.from;
            }

            friend constexpr auto operator<=>(cursor_type const& lhs, cursor_type const& rhs)
                -> std::strong_ordering
                requires ordered_cursor<cursor_t<Base>>
            {
                return lhs.from <=> rhs.from;
            }
        };

    public:
        static constexpr auto first(auto& self) -> cursor_type {
            auto cur = flux::first(self.base_);
            auto end = cur;
            advance(self.base_, end, self.win_sz_ - 1);

            return cursor_type{.from = std::move(cur), .to = std::move(end)};
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.to);
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            flux::inc(self.base_, cur.from);
            flux::inc(self.base_, cur.to);
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(flux::take(flux::slice(self.base_, cur.from, flux::last), self.win_sz_))
        {
            return flux::take(flux::slice(self.base_, cur.from, flux::last), self.win_sz_);
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<Base> && bidirectional_sequence<Base>
        {
            auto end = flux::last(self.base_);
            auto cur = end;
            advance(self.base_, cur, 1 - self.win_sz_);
            return cursor_type{.from = std::move(cur), .to = std::move(end)};
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
            requires bidirectional_sequence<Base>
        {
            flux::dec(self.base_, cur.from);
            flux::dec(self.base_, cur.to);
        }

        static constexpr auto inc(auto& self, cursor_type& cur, distance_t offset) -> void
            requires random_access_sequence<Base>
        {
            flux::inc(self.base_, cur.from, offset);
            flux::inc(self.base_, cur.to, offset);
        }

        static constexpr auto distance(auto& self, cursor_type const& from, cursor_type const& to)
            -> distance_t
            requires random_access_sequence<Base>
        {
            return flux::distance(self.base_, from.from, to.from);
        }

        static constexpr auto size(auto& self) -> distance_t
            requires sized_sequence<Base>
        {
            auto s = (flux::size(self.base_) - self.win_sz_) + 1;
            return std::max(s, distance_t{0});
        }
    };
};

struct slide_fn {
    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, std::integral auto win_sz) const
        -> sequence auto
    {
        return slide_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq),
                                                checked_cast<distance_t>(win_sz));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto slide = detail::slide_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::slide(std::integral auto win_sz) &&
        requires multipass_sequence<D>
{
    FLUX_ASSERT(win_sz > 0);
    return flux::slide(std::move(derived()), std::move(win_sz));
}

} // namespace slide

#endif // FLUX_OP_SLIDE_HPP_INCLUDED


#ifndef FLUX_OP_SORT_HPP_INCLUDED
#define FLUX_OP_SORT_HPP_INCLUDED


// flux/op/detail/pqdsort.hpp
//
// Copyright Orson Peters 2017.
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Modified from Boost.Sort by Orson Peters
// https://github.com/boostorg/sort/blob/develop/include/boost/sort/pdqsort/pdqsort.hpp

#ifndef FLUX_OP_DETAIL_PDQSORT_HPP_INCLUDED
#define FLUX_OP_DETAIL_PDQSORT_HPP_INCLUDED




// flux/op/detail/heap_sift.hpp
//
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Eric Niebler 2014
//  Copyright Casey Carter 2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef FLUX_OP_DETAIL_HEAP_OPS_HPP_INCLUDED
#define FLUX_OP_DETAIL_HEAP_OPS_HPP_INCLUDED



namespace flux::detail {

template <typename Seq, typename Comp>
constexpr void sift_up_n(Seq& seq, distance_t n, Comp& comp)
{
    cursor_t<Seq> first = flux::first(seq);

    if (n > 1) {
        cursor_t<Seq> last = flux::next(seq, first, n);
        n = (n - 2) / 2;
        cursor_t<Seq> i = first + n;
        if (std::invoke(comp, read_at(seq, i), read_at(seq, dec(seq, last)))) {
            value_t<Seq> v = move_at(seq, last);
            do {
                read_at(seq, last) = move_at(seq, i);
                last = i;
                if (n == 0) {
                    break;
                }
                n = (n - 1) / 2;
                i = next(seq, first, n);
            } while (std::invoke(comp, read_at(seq, i), v));
            read_at(seq, last) = std::move(v);
        }
    }
}

template <typename Seq, typename Comp>
constexpr void sift_down_n(Seq& seq, distance_t n, cursor_t<Seq> start,
                           Comp& comp)
{
    cursor_t<Seq> first = flux::first(seq);

    // left-child of start is at 2 * start + 1
    // right-child of start is at 2 * start + 2
    //auto child = start - first;
    auto child = flux::distance(seq, first, start);

    if (n < 2 || (n - 2) / 2 < child) {
        return;
    }

    child = 2 * child + 1;
    cursor_t<Seq> child_i = flux::next(seq, first, child);

    if ((child + 1) < n && std::invoke(comp, read_at(seq, child_i),
                                       read_at(seq, next(seq, child_i)))) {
        // right-child exists and is greater than left-child
        flux::inc(seq, child_i);
        ++child;
    }

    // check if we are in heap-order
    if (std::invoke(comp, read_at(seq, child_i),
                     read_at(seq, start))) {
        // we are, start is larger than its largest child
        return;
    }

    value_t<Seq> top = move_at(seq, start);
    do {
        // we are not in heap-order, swap the parent with it's largest child
        read_at(seq, start) = move_at(seq, child_i);
        //*start = nano::iter_move(child_i);
        start = child_i;

        if ((n - 2) / 2 < child) {
            break;
        }

        // recompute the child based off of the updated parent
        child = 2 * child + 1;
        child_i = next(seq, first, child); //child_i = first + child;

        if ((child + 1) < n &&
            std::invoke(comp, read_at(seq, child_i),
                         read_at(seq, next(seq, child_i)))) {
            // right-child exists and is greater than left-child
            inc(seq, child_i);
            ++child;
        }

        // check if we are in heap-order
    } while (!std::invoke(comp, read_at(seq, child_i), top));
    read_at(seq, start) = std::move(top);
}

template <sequence Seq, typename Comp>
constexpr void make_heap(Seq& seq, Comp& comp)
{
    distance_t n = flux::size(seq);
    auto first = flux::first(seq);

    if (n > 1) {
        for (auto start = (n - 2) / 2; start >= 0; --start) {
            detail::sift_down_n(seq, n, flux::next(seq, first, start), comp);
        }
    }
}

template <sequence Seq, typename Comp>
constexpr void pop_heap(Seq& seq, distance_t n, Comp& comp)
{
    auto first = flux::first(seq);
    if (n > 1) {
        swap_at(seq, first, next(seq, first, n - 1));
        detail::sift_down_n(seq, n - 1, first, comp);
    }
}

template <sequence Seq, typename Comp>
constexpr void sort_heap(Seq& seq, Comp& comp)
{
    auto n = flux::size(seq);

    if (n < 2) {
        return;
    }

    for (auto i = n; i > 1; --i) {
        pop_heap(seq, i, comp);
    }
}

}

#endif


namespace flux {

namespace detail {

// Partitions below this size are sorted using insertion sort.
inline constexpr int pdqsort_insertion_sort_threshold = 24;

// Partitions above this size use Tukey's ninther to select the pivot.
inline constexpr int pdqsort_ninther_threshold = 128;

// When we detect an already sorted partition, attempt an insertion sort that
// allows this amount of element moves before giving up.
inline constexpr int pqdsort_partial_insertion_sort_limit = 8;

// Must be multiple of 8 due to loop unrolling, and < 256 to fit in unsigned
// char.
inline constexpr int pdqsort_block_size = 64;

// Cacheline size, assumes power of two.
inline constexpr int pdqsort_cacheline_size = 64;

template <typename T>
inline constexpr bool is_default_compare_v = false;

template <>
inline constexpr bool is_default_compare_v<std::compare_three_way> = true;
template <>
inline constexpr bool is_default_compare_v<decltype(flux::cmp::reverse_compare)> = true;
template <>
inline constexpr bool is_default_compare_v<decltype(flux::cmp::compare_floating_point_unchecked)> = true;

// Returns floor(log2(n)), assumes n > 0.
template <class T>
constexpr int log2(T n)
{
    int log = 0;
    while (n >>= 1)
        ++log;
    return log;
}

// Sorts [begin, end) using insertion sort with the given comparison function.
template <sequence Seq, typename Comp, typename Cur = cursor_t<Seq>>
constexpr void insertion_sort(Seq& seq, Cur const begin, Cur const end, Comp& comp)
{
    using T = value_t<Seq>;

    if (begin == end) {
        return;
    }

    for (auto cur = next(seq, begin); cur != end; inc(seq, cur)) {
        cursor_t<Seq> sift = cur;
        cursor_t<Seq> sift_1 = prev(seq, cur);

        // Compare first so we can avoid 2 moves for an element already
        // positioned correctly.
        if (comp(read_at(seq, sift), read_at(seq, sift_1))) {
            T tmp = move_at(seq, sift);

            do {
                read_at(seq, sift) = move_at(seq, sift_1);
                dec(seq, sift);
            } while (sift != begin && comp(tmp, read_at(seq, dec(seq, sift_1))));

            read_at(seq, sift) = std::move(tmp);
        }
    }
}

// Sorts [begin, end) using insertion sort with the given comparison function.
// Assumes
// *(begin - 1) is an element smaller than or equal to any element in [begin,
// end).
template <sequence Seq, typename Comp, typename Cur = cursor_t<Seq>>
constexpr void unguarded_insertion_sort(Seq& seq, Cur const begin, Cur const end,
                                        Comp& comp)
{
    using T = value_t<Seq>;

    if (begin == end) {
        return;
    }

    for (auto cur = next(seq, begin); cur != end; inc(seq, cur)) {
        cursor_t<Seq> sift = cur;
        cursor_t<Seq> sift_1 = prev(seq, cur);

        // Compare first so we can avoid 2 moves for an element already
        // positioned correctly.
        if (comp(read_at(seq, sift), read_at(seq, sift_1))) {
            T tmp = move_at(seq, sift);

            do {
                read_at(seq, sift) = move_at(seq, sift_1);
                dec(seq, sift);
            } while (comp(tmp, read_at(seq, dec(seq, sift_1))));

            read_at(seq, sift) = std::move(tmp);
        }
    }
}

// Attempts to use insertion sort on [begin, end). Will return false if more
// than partial_insertion_sort_limit elements were moved, and abort sorting.
// Otherwise it will successfully sort and return true.
template <sequence Seq, typename Comp, typename Cur = cursor_t<Seq>>
constexpr bool partial_insertion_sort(Seq& seq, Cur const begin, Cur const end, Comp& comp)
{
    using T = value_t<Seq>;

    if (begin == end) {
        return true;
    }

    distance_t limit = 0;

    for (auto cur = next(seq, begin); cur != end; inc(seq, cur)) {
        if (limit > pqdsort_partial_insertion_sort_limit) {
            return false;
        }

        cursor_t<Seq> sift = cur;
        cursor_t<Seq> sift_1 = prev(seq, cur);

        // Compare first so we can avoid 2 moves for an element already
        // positioned correctly.
        if (comp(read_at(seq, sift), read_at(seq, sift_1))) {
            T tmp = move_at(seq, sift);

            do {
                read_at(seq, sift) = move_at(seq, sift_1);
                dec(seq, sift);
            } while (sift != begin && comp(tmp, read_at(seq, dec(seq, sift_1))));

            read_at(seq, sift) = std::move(tmp);
            limit += distance(seq, sift, cur);
        }
    }

    return true;
}

template <sequence Seq, typename Comp>
constexpr void sort2(Seq& seq, cursor_t<Seq> a, cursor_t<Seq> b, Comp& comp)
{
    if (comp(read_at(seq, b), read_at(seq, a))) {
        swap_at(seq, a, b);
    }
}

// Sorts the elements *a, *b and *c using comparison function comp.
template <sequence Seq, typename Comp, typename Cur = cursor_t<Seq>>
constexpr void sort3(Seq& seq, Cur a, Cur b, Cur c, Comp& comp)
{
    sort2(seq, a, b, comp);
    sort2(seq, b, c, comp);
    sort2(seq, a, b, comp);
}


template <typename Seq, typename Cur = cursor_t<Seq>>
constexpr void swap_offsets(Seq& seq, Cur const first, Cur const last,
                            unsigned char* offsets_l,
                            unsigned char* offsets_r, int num, bool use_swaps)
{
    using T = value_t<Seq>;
    if (use_swaps) {
        // This case is needed for the descending distribution, where we need
        // to have proper swapping for pdqsort to remain O(n).
        for (int i = 0; i < num; ++i) {
            swap_at(seq, next(seq, first, offsets_l[i]), next(seq, last, -offsets_r[i]));
        }
    } else if (num > 0) {
        Cur l = next(seq, first, offsets_l[0]);
        Cur r = next(seq, last, -offsets_r[0]);
        T tmp(move_at(seq, l));
        read_at(seq, l) = move_at(seq, r);

        for (int i = 1; i < num; ++i) {
            l = next(seq, first, offsets_l[i]);
            read_at(seq, r) = move_at(seq, l);
            r = next(seq, last, -offsets_r[i]);
            read_at(seq, l) = move_at(seq, r);
        }
        read_at(seq, r) = std::move(tmp);
    }
}

// Partitions [begin, end) around pivot *begin using comparison function comp.
// Elements equal to the pivot are put in the right-hand partition. Returns the
// position of the pivot after partitioning and whether the passed sequence
// already was correctly partitioned. Assumes the pivot is a median of at least
// 3 elements and that [begin, end) is at least insertion_sort_threshold long.
// Uses branchless partitioning.
template <typename Seq, typename Cur = cursor_t<Seq>, typename Comp>
constexpr std::pair<Cur, bool>
partition_right_branchless(Seq& seq, Cur const begin, Cur const end, Comp& comp)
{
    using T = value_t<Seq>;

    // Move pivot into local for speed.
    T pivot(move_at(seq, begin));
    Cur first = begin;
    Cur last = end;

    // Find the first element greater than or equal than the pivot (the median
    // of 3 guarantees this exists).
    while (comp(read_at(seq, inc(seq, first)), pivot))
        ;

    // Find the first element strictly smaller than the pivot. We have to guard
    // this search if there was no element before *first.
    if (prev(seq, first) == begin) {
        while (first < last && !comp(read_at(seq, dec(seq, last)), pivot))
            ;
    } else {
        while (!comp(read_at(seq, dec(seq, last)), pivot))
            ;
    }

    // If the first pair of elements that should be swapped to partition are the
    // same element, the passed in sequence already was correctly partitioned.
    bool already_partitioned = first >= last;
    if (!already_partitioned) {
        swap_at(seq, first, last);
        inc(seq, first);
    }

    // The following branchless partitioning is derived from "BlockQuicksort:
    // How Branch Mispredictions don't affect Quicksort" by Stefan Edelkamp and
    // Armin Weiss.
    alignas(pdqsort_cacheline_size) unsigned char
        offsets_l_storage[pdqsort_block_size] = {};
    alignas(pdqsort_cacheline_size) unsigned char
        offsets_r_storage[pdqsort_block_size] = {};
    unsigned char* offsets_l = offsets_l_storage;
    unsigned char* offsets_r = offsets_r_storage;
    int num_l = 0, num_r = 0, start_l = 0, start_r = 0;

    while (distance(seq, first, last) > 2 * pdqsort_block_size) {
        // Fill up offset blocks with elements that are on the wrong side.
        if (num_l == 0) {
            start_l = 0;
            Cur cur = first;
            for (unsigned char i = 0; i < pdqsort_block_size;) {
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
            }
        }
        if (num_r == 0) {
            start_r = 0;
            Cur cur = last;
            for (unsigned char i = 0; i < pdqsort_block_size;) {
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
            }
        }

        // Swap elements and update block sizes and first/last boundaries.
        int num = (std::min)(num_l, num_r);
        swap_offsets(seq, first, last, offsets_l + start_l, offsets_r + start_r, num,
                     num_l == num_r);
        num_l -= num;
        num_r -= num;
        start_l += num;
        start_r += num;
        if (num_l == 0)
            inc(seq, first, pdqsort_block_size);
        if (num_r == 0)
            inc(seq, last, -pdqsort_block_size);
    }

    distance_t l_size = 0, r_size = 0;
    distance_t unknown_left =
        distance(seq, first, last) - ((num_r || num_l) ? pdqsort_block_size : 0);
    if (num_r) {
        // Handle leftover block by assigning the unknown elements to the other
        // block.
        l_size = unknown_left;
        r_size = pdqsort_block_size;
    } else if (num_l) {
        l_size = pdqsort_block_size;
        r_size = unknown_left;
    } else {
        // No leftover block, split the unknown elements in two blocks.
        l_size = unknown_left / 2;
        r_size = unknown_left - l_size;
    }

    // Fill offset buffers if needed.
    if (unknown_left && !num_l) {
        start_l = 0;
        Cur cur = first;
        for (unsigned char i = 0; static_cast<distance_t>(i) < l_size;) {
            offsets_l[num_l] = i++;
            num_l += !comp(read_at(seq, cur), pivot);
            inc(seq, cur);
        }
    }
    if (unknown_left && !num_r) {
        start_r = 0;
        Cur cur = last;
        for (unsigned char i = 0; static_cast<distance_t>(i) < r_size;) {
            offsets_r[num_r] = ++i;
            num_r += comp(read_at(seq, dec(seq, cur)), pivot);
        }
    }

    int num = (std::min)(num_l, num_r);
    swap_offsets(seq, first, last, offsets_l + start_l, offsets_r + start_r, num,
                 num_l == num_r);
    num_l -= num;
    num_r -= num;
    start_l += num;
    start_r += num;
    if (num_l == 0)
        inc(seq, first, l_size);
    if (num_r == 0)
        inc(seq, last, -r_size);

    // We have now fully identified [first, last)'s proper position. Swap the
    // last elements.
    if (num_l) {
        offsets_l += start_l;
        while (num_l--) {
            swap_at(seq, next(seq, first, offsets_l[num_l]), dec(seq, last));
        }
        first = last;
    }
    if (num_r) {
        offsets_r += start_r;
        while (num_r--) {
            swap_at(seq, next(seq, last, -offsets_r[num_r]), first);
            inc(seq, first);
        }
        last = first;
    }

    // Put the pivot in the right place.
    Cur pivot_pos = prev(seq, first);
    read_at(seq, begin) = move_at(seq, pivot_pos);
    read_at(seq, pivot_pos) = std::move(pivot);

    return std::make_pair(std::move(pivot_pos), already_partitioned);
}

// Partitions [begin, end) around pivot *begin using comparison function comp.
// Elements equal to the pivot are put in the right-hand partition. Returns the
// position of the pivot after partitioning and whether the passed sequence
// already was correctly partitioned. Assumes the pivot is a median of at least
// 3 elements and that [begin, end) is at least insertion_sort_threshold long.
template <sequence Seq, typename Comp, typename Cur = cursor_t<Seq>>
constexpr std::pair<cursor_t<Seq>, bool>
partition_right(Seq& seq, Cur const begin, Cur const end, Comp& comp)
{
    using T = value_t<Seq>;

    // Move pivot into local for speed.
    T pivot(move_at(seq, begin));

    cursor_t<Seq> first = begin;
    cursor_t<Seq> last = end;

    // Find the first element greater than or equal than the pivot (the median
    // of 3 guarantees this exists).
    while (comp(read_at(seq, inc(seq, first)), pivot)) {
    }

    // Find the first element strictly smaller than the pivot. We have to guard
    // this search if there was no element before *first.
    if (prev(seq, first) == begin) {
        while (first < last && !comp(read_at(seq, dec(seq, last)), pivot)) {
        }
    } else {
        while (!comp(read_at(seq, dec(seq, last)), pivot)) {
        }
    }

    // If the first pair of elements that should be swapped to partition are the
    // same element, the passed in sequence already was correctly partitioned.
    bool already_partitioned = first >= last;

    // Keep swapping pairs of elements that are on the wrong side of the pivot.
    // Previously swapped pairs guard the searches, which is why the first
    // iteration is special-cased above.
    while (first < last) {
        swap_at(seq, first, last);
        while (comp(read_at(seq, inc(seq, first)), pivot))
            ;
        while (!comp(read_at(seq, dec(seq, last)), pivot))
            ;
    }

    // Put the pivot in the right place.
    cursor_t<Seq> pivot_pos = prev(seq, first);
    read_at(seq, begin) = move_at(seq, pivot_pos);
    read_at(seq, pivot_pos) = std::move(pivot);

    return std::make_pair(std::move(pivot_pos), already_partitioned);
}

// Similar function to the one above, except elements equal to the pivot are put
// to the left of the pivot and it doesn't check or return if the passed
// sequence already was partitioned. Since this is rarely used (the many equal
// case), and in that case pdqsort already has O(n) performance, no block
// quicksort is applied here for simplicity.
template <sequence Seq, typename Comp, typename Cur = cursor_t<Seq>>
constexpr cursor_t<Seq> partition_left(Seq& seq, Cur const begin, Cur const end, Comp& comp)
{
    using T = value_t<Seq>;

    T pivot(move_at(seq, begin));
    cursor_t<Seq> first = begin;
    cursor_t<Seq> last = end;

    while (comp(pivot, read_at(seq, dec(seq, last))))
        ;

    if (next(seq, last) == end) {
        while (first < last && !comp(pivot, read_at(seq, inc(seq, first))))
            ;
    } else {
        while (!comp(pivot, read_at(seq, inc(seq, first))))
            ;
    }

    while (first < last) {
        swap_at(seq, first, last);
        while (comp(pivot, read_at(seq, dec(seq, last))))
            ;
        while (!comp(pivot, read_at(seq, inc(seq, first))))
            ;
    }

    cursor_t<Seq> pivot_pos = last;
    read_at(seq, begin) = move_at(seq, pivot_pos);
    read_at(seq, pivot_pos) = std::move(pivot);

    return pivot_pos;
}

template <bool Branchless, typename Seq, typename Comp,
          typename Cur = cursor_t<Seq>>
constexpr void pdqsort_loop(Seq& seq, Cur begin, Cur end, Comp& comp,
                            int bad_allowed, bool leftmost = true)
{
    using diff_t = distance_t;

    // Use a while loop for tail recursion elimination.
    while (true) {
        diff_t size = flux::distance(seq, begin, end);

        // Insertion sort is faster for small arrays.
        if (size < pdqsort_insertion_sort_threshold) {
            if (leftmost) {
                insertion_sort(seq, begin, end, comp);
            } else {
                unguarded_insertion_sort(seq, begin, end, comp);
            }
            return;
        }

        // Choose pivot as median of 3 or pseudomedian of 9.
        diff_t s2 = size / 2;
        if (size > pdqsort_ninther_threshold) {
            sort3(seq, begin, next(seq, begin, s2), prev(seq, end), comp);
            sort3(seq, next(seq, begin), next(seq, begin, s2 - 1), next(seq, end, -2), comp);
            sort3(seq, next(seq, begin, 2), next(seq, begin, s2 + 1), next(seq, end, -3), comp);
            sort3(seq, next(seq, begin, s2 - 1), next(seq, begin, s2), next(seq, begin, s2 + 1), comp);
            swap_at(seq, begin, next(seq, begin, s2));
        } else {
            sort3(seq, next(seq, begin, s2), begin, prev(seq, end), comp);
        }

        // If *(begin - 1) is the end of the right partition of a previous
        // partition operation there is no element in [begin, end) that is
        // smaller than *(begin - 1). Then if our pivot compares equal to
        // *(begin - 1) we change strategy, putting equal elements in the left
        // partition, greater elements in the right partition. We do not have to
        // recurse on the left partition, since it's sorted (all equal).
        if (!leftmost && !comp(read_at(seq, prev(seq, begin)), read_at(seq, begin))) {
            begin = next(seq, partition_left(seq, begin, end, comp));
            continue;
        }

        // Partition and get results.
        auto [pivot_pos, already_partitioned] = [&] {
            if constexpr (Branchless) {
                return partition_right_branchless(seq, begin, end, comp);
            } else {
                return partition_right(seq, begin, end, comp);
            }
        }();

        // Check for a highly unbalanced partition.
        diff_t l_size = distance(seq, begin, pivot_pos);
        diff_t r_size = distance(seq, next(seq, pivot_pos), end);
        bool highly_unbalanced = l_size < size / 8 || r_size < size / 8;

        // If we got a highly unbalanced partition we shuffle elements to break
        // many patterns.
        if (highly_unbalanced) {
            // If we had too many bad partitions, switch to heapsort to
            // guarantee O(n log n).
            if (--bad_allowed == 0) {
                auto subseq = flux::slice(seq, begin, end);
                detail::make_heap(subseq, comp);
                detail::sort_heap(subseq, comp);
                return;
            }

            if (l_size >= pdqsort_insertion_sort_threshold) {
                swap_at(seq, begin, next(seq, begin, l_size/4));
                swap_at(seq, prev(seq, pivot_pos), next(seq, pivot_pos, -l_size/4));

                if (l_size > pdqsort_ninther_threshold) {
                    swap_at(seq, next(seq, begin), next(seq, begin, l_size/4 + 1));
                    swap_at(seq, next(seq, begin, 2), next(seq, begin, l_size/4 + 2));
                    swap_at(seq, next(seq, pivot_pos, -2), next(seq, pivot_pos, -(l_size/4 + 1)));
                    swap_at(seq, next(seq, pivot_pos, -3), next(seq, pivot_pos, -(l_size/4 + 2)));
                }
            }

            if (r_size >= pdqsort_insertion_sort_threshold) {
                swap_at(seq, next(seq, pivot_pos), next(seq, pivot_pos, (1 + r_size/4)));
                swap_at(seq, prev(seq, end), next(seq, end, -r_size/4));

                if (r_size > pdqsort_ninther_threshold) {
                    swap_at(seq, next(seq, pivot_pos, 2), next(seq, pivot_pos, 2 + r_size/4));
                    swap_at(seq, next(seq, pivot_pos, 3), next(seq, pivot_pos, 3 + r_size/4));
                    swap_at(seq, next(seq, end, -2), next(seq, end, -(1 + r_size/4)));
                    swap_at(seq, next(seq, end, -3), next(seq, end, -(2 + r_size/4)));
                }
            }
        } else {
            // If we were decently balanced and we tried to sort an already
            // partitioned sequence try to use insertion sort.
            if (already_partitioned &&
                partial_insertion_sort(seq, begin, pivot_pos, comp) &&
                partial_insertion_sort(seq, flux::next(seq, pivot_pos), end, comp))
                return;
        }

        // Sort the left partition first using recursion and do tail recursion
        // elimination for the right-hand partition.
        detail::pdqsort_loop<Branchless>(seq, begin, pivot_pos, comp,
                                         bad_allowed, leftmost);
        begin = next(seq, pivot_pos);
        leftmost = false;
    }
}

template <typename Seq, typename Comp>
constexpr void pdqsort(Seq& seq, Comp& comp)
{
    if (is_empty(seq)) {
        return;
    }

    constexpr bool Branchless =
         is_default_compare_v<std::remove_const_t<Comp>> &&
         std::is_arithmetic_v<value_t<Seq>>;

    auto comp_wrapper = [&comp](auto&& lhs, auto&& rhs) -> bool {
        return std::is_lt(std::invoke(comp, FLUX_FWD(lhs), FLUX_FWD(rhs)));
    };

    detail::pdqsort_loop<Branchless>(seq,
                                     first(seq),
                                     last(seq),
                                     comp_wrapper,
                                     detail::log2(size(seq)));
}

} // namespace detail

} // namespace flux

#endif


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_UNCHECKED_HPP_INCLUDED
#define FLUX_OP_UNCHECKED_HPP_INCLUDED



namespace flux {

namespace detail {

template <sequence Base>
struct unchecked_adaptor : inline_sequence_base<unchecked_adaptor<Base>> {
private:
    Base base_;

public:
    constexpr explicit unchecked_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    constexpr auto base() & -> Base& { return base_; }
    constexpr auto base() const& -> Base const& { return base_; }

    struct flux_sequence_traits : passthrough_traits_base<Base> {

        using value_type = value_t<Base>;
        static constexpr bool disable_multipass = !multipass_sequence<Base>;
        static constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto read_at(auto& self, auto const& cur)
            -> element_t<Base>
        {
            return flux::read_at_unchecked(self.base(), cur);
        }

        static constexpr auto move_at(auto& self, auto const& cur)
            -> rvalue_element_t<Base>
        {
            return flux::move_at_unchecked(self.base(), cur);
        }
    };
};

struct unchecked_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
        -> unchecked_adaptor<std::decay_t<Seq>>
    {
        return unchecked_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto unchecked = detail::unchecked_fn{};

} // namespace flux

#endif // FLUX_OP_UNCHECKED_HPP_INCLUDED


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
constexpr void inline_sequence_base<D>::sort(Cmp cmp)
{
    return flux::sort(derived(), std::ref(cmp));
}

} // namespace flux

#endif // FLUX_OP_SORT_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SPLIT_HPP_INCLUDED
#define FLUX_OP_SPLIT_HPP_INCLUDED





// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SEARCH_HPP_INCLUDED
#define FLUX_OP_SEARCH_HPP_INCLUDED



namespace flux {

namespace detail {

struct search_fn {
    template <multipass_sequence Haystack, multipass_sequence Needle,
              typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, element_t<Haystack>, element_t<Needle>>
    constexpr auto operator()(Haystack&& h, Needle&& n, Cmp cmp = {}) const
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

                if (!std::invoke(cmp, read_at(h, cur1), read_at(n, cur2))) {
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

FLUX_EXPORT inline constexpr auto search = detail::search_fn{};

} // namespace flux

#endif





namespace flux {

namespace detail {

template <typename Splitter, typename Seq>
concept splitter_for = requires(Splitter& splitter, Seq& seq, cursor_t<Seq> const& cur) {
    { splitter(flux::slice(seq, cur, flux::last)) } -> std::same_as<bounds_t<Seq>>;
};

template <multipass_sequence Base, splitter_for<Base> Splitter>
struct split_adaptor : inline_sequence_base<split_adaptor<Base, Splitter>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Splitter splitter_;

public:
    constexpr split_adaptor(decays_to<Base> auto&& base, decays_to<Splitter> auto&& splitter)
        : base_(FLUX_FWD(base)),
          splitter_(FLUX_FWD(splitter))
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> cur{};
            bounds_t<Base> next{};
            bool trailing_empty = false;

            friend constexpr bool operator==(cursor_type const& lhs, cursor_type const& rhs)
            {
                return lhs.cur == rhs.cur && lhs.trailing_empty == rhs.trailing_empty;
            }
        };

    public:
        static constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto first(auto& self) -> cursor_type
            requires sequence<decltype((self.base_))> &&
                     splitter_for<decltype((self.splitter_)), decltype((self.base_))>
        {
            auto fst = flux::first(self.base_);
            auto bounds = self.splitter_(flux::slice(self.base_, fst, flux::last));
            return cursor_type{.cur = std::move(fst),
                               .next = std::move(bounds)};
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur)
            -> bool
        {
            return flux::is_last(self.base_, cur.cur) && !cur.trailing_empty;
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
        {
            return flux::slice(self.base_, cur.cur, cur.next.from);
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            cur.cur = cur.next.from;
            if (!flux::is_last(self.base_, cur.cur)) {
                cur.cur = cur.next.to;
                if (flux::is_last(self.base_, cur.cur)) {
                    cur.trailing_empty = true;
                    cur.next = {cur.cur, cur.cur};
                } else {
                    cur.next = self.splitter_(flux::slice(self.base_, cur.cur, flux::last));
                }
            } else {
                cur.trailing_empty = false;
            }
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<decltype(self.base_)>
        {
            return cursor_type{.cur = flux::last(self.base_)};
        }
    };
};

template <multipass_sequence Pattern>
struct pattern_splitter {
private:
    FLUX_NO_UNIQUE_ADDRESS Pattern pattern_;

public:
    constexpr explicit pattern_splitter(decays_to<Pattern> auto&& pattern)
        : pattern_(FLUX_FWD(pattern))
    {}

    template <multipass_sequence Seq>
        requires std::equality_comparable_with<element_t<Seq>, element_t<Pattern>>
    constexpr auto operator()(Seq&& seq) -> bounds_t<Seq>
    {
        return flux::search(seq, pattern_);
    }

    template <multipass_sequence Seq>
        requires multipass_sequence<Pattern const> &&
                 std::equality_comparable_with<element_t<Seq>, element_t<Pattern const>>
    constexpr auto operator()(Seq&& seq) const -> bounds_t<Seq>
    {
        return flux::search(seq, pattern_);
    }
};

template <typename Delim>
struct delim_splitter {
private:
    FLUX_NO_UNIQUE_ADDRESS Delim delim_;

public:
    constexpr explicit delim_splitter(decays_to<Delim> auto&& delim)
        : delim_(FLUX_FWD(delim))
    {}

    template <multipass_sequence Seq>
        requires std::equality_comparable_with<element_t<Seq>, Delim const&>
    constexpr auto operator()(Seq&& seq) const -> bounds_t<Seq>
    {
        auto nxt = flux::find(seq, delim_);
        if (!flux::is_last(seq, nxt)) {
            return bounds{nxt, flux::next(seq, nxt)};
        } else {
            return bounds{nxt, nxt};
        }
    }
};

template <typename Pred>
struct predicate_splitter {
private:
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;

public:
    constexpr explicit predicate_splitter(decays_to<Pred> auto&& pred)
        : pred_(FLUX_FWD(pred))
    {}

    template <multipass_sequence Seq>
        requires std::predicate<Pred const&, element_t<Seq>>
    constexpr auto operator()(Seq&& seq) const -> bounds_t<Seq>
    {
        auto nxt = flux::find_if(seq, pred_);
        if (!flux::is_last(seq, nxt)) {
            return bounds{nxt, flux::next(seq, nxt)};
        } else {
            return bounds{nxt, nxt};
        }
    }
};

struct split_fn {
    template <adaptable_sequence Seq, adaptable_sequence Pattern>
        requires multipass_sequence<Seq> &&
                 multipass_sequence<Pattern> &&
                 std::equality_comparable_with<element_t<Seq>, element_t<Pattern>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pattern&& pattern) const
    {
        using splitter_t = pattern_splitter<std::decay_t<Pattern>>;
        return split_adaptor<std::decay_t<Seq>, splitter_t>(
            FLUX_FWD(seq), splitter_t(FLUX_FWD(pattern)));
    }

    template <adaptable_sequence Seq, typename Delim>
        requires multipass_sequence<Seq> &&
                 std::equality_comparable_with<element_t<Seq>, Delim const&>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Delim&& delim) const
    {
        using splitter_t = delim_splitter<std::decay_t<Delim>>;
        return split_adaptor<std::decay_t<Seq>, splitter_t>(
            FLUX_FWD(seq), splitter_t(FLUX_FWD(delim)));
    }

    template <adaptable_sequence Seq, typename Pred>
        requires multipass_sequence<Seq> &&
                 std::predicate<Pred const&, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred) const
    {
        using splitter_t = predicate_splitter<Pred>;
        return split_adaptor<std::decay_t<Seq>, splitter_t>(
            FLUX_FWD(seq), splitter_t(std::move(pred)));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto split = detail::split_fn{};

template <typename Derived>
template <typename Pattern>
    requires multipass_sequence<Derived> &&
             multipass_sequence<Pattern> &&
             std::equality_comparable_with<element_t<Derived>, element_t<Pattern>>
constexpr auto inline_sequence_base<Derived>::split(Pattern&& pattern) &&
{
    return flux::split(std::move(derived()), FLUX_FWD(pattern));
}

template <typename Derived>
template <typename Delim>
    requires multipass_sequence<Derived> &&
             std::equality_comparable_with<element_t<Derived>, Delim const&>
constexpr auto inline_sequence_base<Derived>::split(Delim&& delim) &&
{
    return flux::split(std::move(derived()), FLUX_FWD(delim));
}

template <typename Derived>
template <typename Pred>
    requires multipass_sequence<Derived> &&
             std::predicate<Pred const&, element_t<Derived>>
constexpr auto inline_sequence_base<Derived>::split(Pred pred) &&
{
    return flux::split(std::move(derived()), std::move(pred));
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

template <typename C>
concept character = any_of<C, char, wchar_t, char8_t, char16_t, char32_t>;

struct to_string_view_fn {
    template <contiguous_sequence Seq>
        requires sized_sequence<Seq> && character<value_t<Seq>>
    constexpr auto operator()(Seq&& seq) const
    {
        return std::basic_string_view<value_t<Seq>>(flux::data(seq), flux::usize(seq));
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

FLUX_EXPORT inline constexpr auto split_string = detail::split_string_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::split_string(auto&& pattern) &&
{
    return flux::split_string(std::move(derived()), FLUX_FWD(pattern));
}


} // namespace flux

#endif


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_STARTS_WITH_HPP_INCLUDED
#define FLUX_OP_STARTS_WITH_HPP_INCLUDED



namespace flux {

namespace detail {

struct starts_with_fn {
    template <sequence Haystack, sequence Needle, typename Cmp = std::ranges::equal_to>
        requires std::predicate<Cmp&, element_t<Haystack>, element_t<Needle>>
    constexpr auto operator()(Haystack&& haystack, Needle&& needle, Cmp cmp = Cmp{}) const -> bool
    {
        if constexpr (sized_sequence<Haystack> && sized_sequence<Needle>) {
            if (flux::size(haystack) < flux::size(needle)) {
                return false;
            }
        }

        auto h = flux::first(haystack);
        auto n = flux::first(needle);

        while (!flux::is_last(haystack, h) && !flux::is_last(needle, n)) {
            if (!std::invoke(cmp, flux::read_at(haystack, h), flux::read_at(needle, n))) {
                return false;
            }
            flux::inc(haystack, h);
            flux::inc(needle, n);
        }

        return flux::is_last(needle, n);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto starts_with = detail::starts_with_fn{};

template <typename Derived>
template <sequence Needle, typename Cmp>
    requires std::predicate<Cmp&, element_t<Derived>, element_t<Needle>>
constexpr auto inline_sequence_base<Derived>::starts_with(Needle&& needle, Cmp cmp) -> bool
{
    return flux::starts_with(derived(), FLUX_FWD(needle), std::move(cmp));
}


} // namespace flux

#endif // FLUX_OP_STARTS_WITH_HPP_INCLUDED



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

FLUX_EXPORT inline constexpr auto swap_elements = detail::swap_elements_fn{};

}

#endif



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_TAKE_WHILE_HPP_INCLUDED
#define FLUX_OP_TAKE_WHILE_HPP_INCLUDED




namespace flux {

namespace detail {

template <sequence Base, typename Pred>
struct take_while_adaptor : inline_sequence_base<take_while_adaptor<Base, Pred>> {
private:
    Base base_;
    Pred pred_;

    constexpr auto base() & -> Base& { return base_; }

    friend struct sequence_traits<take_while_adaptor>;
    friend struct passthrough_traits_base<Base>;

public:
    constexpr take_while_adaptor(decays_to<Base> auto&& base, decays_to<Pred> auto&& pred)
        : base_(FLUX_FWD(base)),
          pred_(FLUX_FWD(pred))
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base { return std::move(base_); }
};

struct take_while_fn {
    template <adaptable_sequence Seq, std::move_constructible Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred) const
    {
        return take_while_adaptor<std::decay_t<Seq>, Pred>(
                    FLUX_FWD(seq), std::move(pred));
    }
};

} // namespace detail

template <typename Base, typename Pred>
struct sequence_traits<detail::take_while_adaptor<Base, Pred>>
    : detail::passthrough_traits_base<Base>
{
    using self_t = detail::take_while_adaptor<Base, Pred>;

    using value_type = value_t<Base>;

    static constexpr bool is_infinite = false;

    template <typename Self>
    static constexpr bool is_last(Self& self, cursor_t<Self> const& cur)
        requires std::predicate<decltype((self.pred_)), element_t<decltype(self.base_)>>
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

FLUX_EXPORT inline constexpr auto take_while = detail::take_while_fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::take_while(Pred pred) &&
{
    return flux::take_while(std::move(derived()), std::move(pred));
}

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_TO_HPP_INCLUDED
#define FLUX_OP_TO_HPP_INCLUDED




// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_OUTPUT_TO_HPP_INCLUDED
#define FLUX_OP_OUTPUT_TO_HPP_INCLUDED



#include <cstring>
#include <iterator>

namespace flux {

namespace detail {

struct output_to_fn {
private:
    template <typename Seq, typename Iter>
    static constexpr auto impl(Seq& seq, Iter& iter) -> Iter
    {
        flux::for_each(seq, [&iter](auto&& elem) {
            *iter = FLUX_FWD(elem);
            ++iter;
        });
        return iter;
    }

public:
    template <sequence Seq, std::weakly_incrementable Iter>
        requires std::indirectly_writable<Iter, element_t<Seq>>
    constexpr auto operator()(Seq&& seq, Iter iter) const -> Iter
    {
        constexpr bool can_memcpy =
            contiguous_sequence<Seq> &&
            sized_sequence<Seq> &&
            std::contiguous_iterator<Iter> &&
            std::is_trivially_copyable_v<value_t<Seq>>;

        if constexpr (can_memcpy) {
            if (std::is_constant_evaluated()) {
                return impl(seq, iter);
            } else {
                auto size = flux::usize(seq);
                if (size == 0) {
                    return iter;
                }
                FLUX_ASSERT(flux::data(seq) != nullptr);
                std::memmove(std::to_address(iter), flux::data(seq),
                             size * sizeof(value_t<Seq>));
                return iter + checked_cast<std::iter_difference_t<Iter>>(flux::size(seq));
            }
        } else {
            return impl(seq, iter);
        }
    }
};

}

FLUX_EXPORT inline constexpr auto output_to = detail::output_to_fn{};

template <typename D>
template <typename Iter>
    requires std::weakly_incrementable<Iter> &&
             std::indirectly_writable<Iter, element_t<D>>
constexpr auto inline_sequence_base<D>::output_to(Iter iter) -> Iter
{
    return flux::output_to(derived(), std::move(iter));
}

}

#endif


namespace flux {

FLUX_EXPORT
struct from_sequence_t {
    explicit from_sequence_t() = default;
};

FLUX_EXPORT inline constexpr auto from_sequence = from_sequence_t{};

namespace detail {

template <typename C, typename Seq, typename... Args>
concept direct_sequence_constructible =
    std::constructible_from<C, Seq, Args...>;

template <typename C, typename Seq, typename... Args>
concept from_sequence_constructible =
    std::constructible_from<C, from_sequence_t, Seq, Args...>;

template <typename C>
using container_value_t = typename C::value_type; // Let's just assume it exists

template <sequence Seq>
using common_iterator_t =
    std::ranges::iterator_t<decltype(std::views::common(FLUX_DECLVAL(Seq)))>;


template <typename C, typename Seq, typename... Args>
concept cpp17_range_constructible =
    std::constructible_from<C, common_iterator_t<Seq>, common_iterator_t<Seq>, Args...>;

template <typename C, typename Elem>
concept container_insertable =
    requires (C& c, Elem&& elem) {
        requires (requires { c.push_back(FLUX_FWD(elem)); } ||
                  requires { c.insert(c.end(), FLUX_FWD(elem)); });
    };

template <typename C, typename Seq, typename... Args>
concept container_convertible =
    direct_sequence_constructible<C, Seq, Args...> ||
    from_sequence_constructible<C, Seq, Args...> ||
    cpp17_range_constructible<C, Seq, Args...> ||
    (  std::constructible_from<C, Args...> &&
        container_insertable<C, element_t<Seq>>);

template <typename C>
concept reservable_container =
    std::ranges::sized_range<C> &&
    requires (C& c, std::ranges::range_size_t<C> sz) {
        c.reserve(sz);
        { c.max_size() } -> std::same_as<std::ranges::range_size_t<C>>;
        { c.capacity() } -> std::same_as<std::ranges::range_size_t<C>>;
    };

template <typename Elem, typename C>
constexpr auto make_inserter(C& c)
{
    if constexpr (requires { c.push_back(FLUX_DECLVAL(Elem)); }) {
        return std::back_inserter(c);
    } else {
        return std::inserter(c, c.end());
    }
}

template <template <typename...> typename C, typename Seq, typename... Args>
using ctad_direct_seq = decltype(C(FLUX_DECLVAL(Seq), FLUX_DECLVAL(Args)...));

template <template <typename...> typename C, typename Seq, typename... Args>
using ctad_from_seq = decltype((C(from_sequence, FLUX_DECLVAL(Seq), FLUX_DECLVAL(Args)...)));

template <template <typename...> typename C, typename Seq, typename... Args>
using ctad_from_iters = decltype(C(FLUX_DECLVAL(common_iterator_t<Seq>), FLUX_DECLVAL(common_iterator_t<Seq>), FLUX_DECLVAL(Args)...));

template <template <typename...> typename C, typename Seq, typename... Args>
concept can_deduce_container_type =
    requires { typename ctad_direct_seq<C, Seq, Args...>; } ||
    requires { typename ctad_from_seq<C, Seq, Args...>; } ||
    requires { typename ctad_from_iters<C, Seq, Args...>; } ||
    ( sizeof...(Args) == 0 && requires { typename C<value_t<Seq>>; });

template <typename T>
struct type_t { using type = T; };

template <template <typename...> typename C, typename Seq, typename... Args>
    requires can_deduce_container_type<C, Seq, Args...>
consteval auto deduce_container_type()
{
    if constexpr (requires { typename ctad_direct_seq<C, Seq, Args...>; }) {
        return type_t<decltype(C(FLUX_DECLVAL(Seq), FLUX_DECLVAL(Args)...))>{};
    } else if constexpr (requires { typename ctad_from_seq<C, Seq, Args...>; }) {
        return type_t<decltype((C(from_sequence, FLUX_DECLVAL(Seq), FLUX_DECLVAL(Args)...)))>{};
    } else if constexpr (requires { typename ctad_from_iters<C, Seq, Args...>; }) {
        using I = common_iterator_t<Seq>;
        return type_t<decltype(C(FLUX_DECLVAL(I), FLUX_DECLVAL(I), FLUX_DECLVAL(Args)...))>{};
    } else {
        static_assert(requires { typename C<value_t<Seq>>; });
        return type_t<C<value_t<Seq>>>{};
    }
}

template <template <typename...> typename C, typename Seq, typename... Args>
    requires can_deduce_container_type<C, Seq, Args...>
using deduced_container_t = typename decltype(deduce_container_type<C, Seq, Args...>())::type;


} // namespace detail

FLUX_EXPORT
template <typename Container, sequence Seq, typename... Args>
    requires (std::convertible_to<element_t<Seq>, detail::container_value_t<Container>> &&
                 detail::container_convertible<Container, Seq, Args...>) ||
             sequence<element_t<Seq>>
constexpr auto to(Seq&& seq, Args&&... args) -> Container
{
    if constexpr (std::convertible_to<element_t<Seq>, detail::container_value_t<Container>>) {
        if constexpr (detail::direct_sequence_constructible<Container, Seq, Args...>) {
            return Container(FLUX_FWD(seq), FLUX_FWD(args)...);
        } else if constexpr (detail::from_sequence_constructible<Container, Seq, Args...>) {
            return Container(from_sequence, FLUX_FWD(seq), FLUX_FWD(args)...);
        } else if constexpr (detail::cpp17_range_constructible<Container, Seq, Args...>) {
            auto view_ = std::views::common(FLUX_FWD(seq));
            return Container(view_.begin(), view_.end(), FLUX_FWD(args)...);
        } else {
            auto c = Container(FLUX_FWD(args)...);
            if constexpr (sized_sequence<Seq> && detail::reservable_container<Container>) {
                c.reserve(flux::usize(seq));
            }
            flux::output_to(seq, detail::make_inserter<element_t<Seq>>(c));
            return c;
        }
    } else {
        static_assert(sequence<element_t<Seq>>);
        return flux::to<Container>(flux::map(flux::from_fwd_ref(FLUX_FWD(seq)), [](auto&& elem) {
            return flux::to<detail::container_value_t<Container>>(FLUX_FWD(elem));
        }), FLUX_FWD(args)...);
    }
}

FLUX_EXPORT
template <template <typename...> typename Container, sequence Seq, typename... Args>
    requires detail::can_deduce_container_type<Container, Seq, Args...> &&
             detail::container_convertible<
                 detail::deduced_container_t<Container, Seq, Args...>, Seq, Args...>
constexpr auto to(Seq&& seq, Args&&... args)
{
    using C_ = detail::deduced_container_t<Container, Seq, Args...>;
    return flux::to<C_>(FLUX_FWD(seq), FLUX_FWD(args)...);
}


template <typename D>
template <typename Container, typename... Args>
constexpr auto inline_sequence_base<D>::to(Args&&... args) -> Container
{
    return flux::to<Container>(derived(), FLUX_FWD(args)...);
}

template <typename D>
template <template <typename...> typename Container, typename... Args>
constexpr auto inline_sequence_base<D>::to(Args&&... args)
{
    return flux::to<Container>(derived(), FLUX_FWD(args)...);
}

} // namespace flux

#endif // FLUX_OP_TO_HPP_INCLUDED



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_WRITE_TO_HPP_INCLUDED
#define FLUX_OP_WRITE_TO_HPP_INCLUDED



#include <iosfwd>

namespace flux {

namespace detail {

struct write_to_fn {

    template <sequence Seq, typename OStream>
        requires std::derived_from<OStream, std::ostream>
    auto operator()(Seq&& seq, OStream& os) const
        -> std::ostream&
    {
        bool first = true;
        os << '[';

        flux::for_each(FLUX_FWD(seq), [&os, &first](auto&& elem) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }

            if constexpr (sequence<element_t<Seq>>) {
                write_to_fn{}(FLUX_FWD(elem), os);
            } else {
                os << FLUX_FWD(elem);
            }
        });

        os << ']';
        return os;
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto write_to = detail::write_to_fn{};

template <typename Derived>
auto inline_sequence_base<Derived>::write_to(std::ostream& os) -> std::ostream&
{
    return flux::write_to(derived(), os);
}

} // namespace flux

#endif // FLUX_OP_WRITE_TO_HPP_INCLUDED


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ZIP_ALGORITHMS_HPP_INCLUDED
#define FLUX_OP_ZIP_ALGORITHMS_HPP_INCLUDED





namespace flux {

namespace detail {

struct zip_for_each_while_fn {
    template <typename Pred, sequence... Seqs>
        requires std::invocable<Pred&, element_t<Seqs>...> &&
                 boolean_testable<std::invoke_result_t<Pred&, element_t<Seqs>...>>
    constexpr auto operator()(Pred pred, Seqs&&... seqs) const
        -> std::tuple<cursor_t<Seqs>...>
    {
        if constexpr (sizeof...(Seqs) == 0) {
            return std::tuple<>{};
        } else if constexpr (sizeof...(Seqs) == 1) {
            return std::tuple<cursor_t<Seqs>...>(flux::for_each_while(seqs..., std::ref(pred)));
        } else {
            return [&pred, &...seqs = seqs, ...curs = flux::first(seqs)]() mutable {
                while (!(flux::is_last(seqs, curs) || ...)) {
                    if (!std::invoke(pred, flux::read_at_unchecked(seqs, curs)...)) {
                        break;
                    }
                    (flux::inc(seqs, curs), ...);
                }
                return std::tuple<cursor_t<Seqs>...>(std::move(curs)...);
            }();
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto zip_for_each_while = detail::zip_for_each_while_fn{};

namespace detail {

struct zip_for_each_fn {
    template <std::move_constructible Func, sequence... Seqs>
        requires std::invocable<Func&, element_t<Seqs>...>
    constexpr auto operator()(Func func, Seqs&&... seqs) const -> Func
    {
        zip_for_each_while([&func](auto&&... elems) {
            std::invoke(func, FLUX_FWD(elems)...);
            return true;
        }, seqs...);
        return func;
    }
};

struct zip_find_if_fn {
    template <typename Pred, sequence... Seqs>
        requires std::predicate<Pred&, element_t<Seqs>...>
    constexpr auto operator()(Pred pred, Seqs&&... seqs) const
        -> std::tuple<cursor_t<Seqs>...>
    {
        return zip_for_each_while(std::not_fn(pred), seqs...);
    }
};

template <typename Func, typename Init, typename... Seqs>
using zip_fold_result_t = std::decay_t<std::invoke_result_t<Func&, Init, element_t<Seqs>...>>;

template <typename Func, typename Init, typename R, typename... Seqs>
concept zip_foldable =
    std::invocable<Func&, R, element_t<Seqs>...> &&
    std::convertible_to<Init, R> &&
    std::assignable_from<R&, std::invoke_result_t<Func&, R, element_t<Seqs>...>>;

struct zip_fold_fn {
    template <typename Func, std::movable Init, sequence... Seqs,
              typename R = zip_fold_result_t<Func, Init, Seqs...>>
        requires zip_foldable<Func, Init, R, Seqs...>
    constexpr auto operator()(Func func, Init init, Seqs&&... seqs) const -> R
    {
        R init_ = R(std::move(init));
        zip_for_each_while([&func, &init_](auto&&... elems) {
            init_ = std::invoke(func, std::move(init_), FLUX_FWD(elems)...);
            return true;
        }, seqs...);
        return init_;
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto zip_for_each = detail::zip_for_each_fn{};
FLUX_EXPORT inline constexpr auto zip_find_if = detail::zip_find_if_fn{};
FLUX_EXPORT inline constexpr auto zip_fold = detail::zip_fold_fn{};

} // namespace pred

#endif



// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_ARRAY_PTR_HPP_INCLUDED
#define FLUX_SOURCE_ARRAY_PTR_HPP_INCLUDED





namespace flux {

namespace detail {

template <typename From, typename To>
concept non_slicing_ptr_convertible = std::convertible_to<From (*)[], To (*)[]>;

struct make_array_ptr_unchecked_fn;

}

FLUX_EXPORT
template <typename T>
    requires (std::is_object_v<T> && !std::is_abstract_v<T>)
struct array_ptr : inline_sequence_base<array_ptr<T>> {
private:
    T* data_ = nullptr;
    distance_t sz_ = 0;

    friend struct detail::make_array_ptr_unchecked_fn;

    constexpr array_ptr(T* ptr, distance_t sz) noexcept
        : data_(ptr),
          sz_(sz)
    {}

public:
    array_ptr() = default;

    template <typename U>
        requires (!std::same_as<U, T> &&
                  detail::non_slicing_ptr_convertible<U, T>)
    constexpr explicit(false) array_ptr(array_ptr<U> const& other) noexcept
        : data_(flux::data(other)),
          sz_(flux::size(other))
    {}

    template <typename Seq>
        requires (!decays_to<Seq, array_ptr> &&
                  contiguous_sequence<Seq> &&
                  sized_sequence<Seq> &&
                  detail::non_slicing_ptr_convertible<std::remove_reference_t<element_t<Seq>>, T>)
    constexpr explicit array_ptr(Seq& seq)
        : data_(flux::data(seq)),
          sz_(flux::size(seq))
    {}

    template <typename Seq>
        requires (contiguous_sequence<Seq> &&
                  sized_sequence<Seq> &&
                  detail::non_slicing_ptr_convertible<std::remove_reference_t<element_t<Seq>>, T>)
    constexpr array_ptr(detail::ref_adaptor<Seq> ref)
        : data_(flux::data(ref)),
          sz_(flux::size(ref))
    {}

    array_ptr(array_ptr&&) = default;
    array_ptr& operator=(array_ptr&&) = default;
    ~array_ptr() = default;

    array_ptr(array_ptr const&) requires std::is_const_v<T> = default;
    array_ptr& operator=(array_ptr const&) requires std::is_const_v<T> = default;

    friend constexpr auto operator==(array_ptr const& lhs, array_ptr const&  rhs) -> bool
    {
        return std::ranges::equal_to{}(lhs.data_, rhs.data_) && lhs.sz_ == rhs.sz_;
    }

    struct flux_sequence_traits : default_sequence_traits {

        static constexpr auto first(array_ptr const&) -> index_t { return 0; }

        static constexpr auto is_last(array_ptr const& self, index_t idx) -> bool
        {
            return idx >= self.sz_;
        }

        static constexpr auto inc(array_ptr const& self, index_t& idx) -> void
        {
            FLUX_DEBUG_ASSERT(idx < self.sz_);
            idx = num::checked_add(idx, distance_t{1});
        }

        static constexpr auto read_at(array_ptr const& self, index_t idx) -> T&
        {
            indexed_bounds_check(idx, self.sz_);
            return self.data_[idx];
        }

        static constexpr auto read_at_unchecked(array_ptr const& self, index_t idx) -> T&
        {
            return self.data_[idx];
        }

        static constexpr auto dec(array_ptr const& , index_t& idx) -> void
        {
            FLUX_DEBUG_ASSERT(idx > 0);
            --idx;
        }

        static constexpr auto last(array_ptr const& self) -> index_t { return self.sz_; }

        static constexpr auto inc(array_ptr const& self, index_t& idx, distance_t offset)
            -> void
        {
            index_t nxt = num::checked_add(idx, offset);
            FLUX_DEBUG_ASSERT(nxt >= 0);
            FLUX_DEBUG_ASSERT(nxt <= self.sz_);
            idx = nxt;
        }

        static constexpr auto distance(array_ptr const&, index_t from, index_t to)
            -> distance_t
        {
            return num::checked_sub(to, from);
        }

        static constexpr auto size(array_ptr const& self) -> distance_t
        {
            return self.sz_;
        }

        static constexpr auto data(array_ptr const& self) -> T* { return self.data_; }

        static constexpr auto for_each_while(array_ptr const& self, auto&& pred)
        {
            index_t idx = 0;
            for (; idx < self.sz_; idx++) {
                if (!std::invoke(pred, self.data_[idx])) {
                    break;
                }
            }
            return idx;
        }
    };
};

template <contiguous_sequence Seq>
array_ptr(Seq&) -> array_ptr<std::remove_reference_t<element_t<Seq>>>;

template <contiguous_sequence Seq>
array_ptr(detail::ref_adaptor<Seq>) -> array_ptr<std::remove_reference_t<element_t<Seq>>>;

namespace detail {

struct make_array_ptr_unchecked_fn {
    template <typename T>
        requires (std::is_object_v<T> && !std::is_abstract_v<T>)
    constexpr auto operator()(T* ptr, std::integral auto size) const -> array_ptr<T>
    {
        return array_ptr<T>(ptr, checked_cast<distance_t>(size));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto make_array_ptr_unchecked =
    detail::make_array_ptr_unchecked_fn{};

} // namespace flux

#endif


// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_BITSET_HPP_INCLUDED
#define FLUX_SOURCE_BITSET_HPP_INCLUDED



#include <bitset>

namespace flux {

template <std::size_t N>
struct sequence_traits<std::bitset<N>> : default_sequence_traits {

    using value_type = bool;

    using self_t = std::bitset<N>;

    static constexpr auto first(self_t const&) -> std::size_t { return 0; }

    static constexpr auto is_last(self_t const&, std::size_t idx) { return idx == N; }

    static constexpr auto read_at(self_t& self, std::size_t idx)
        -> typename std::bitset<N>::reference
    {
        return self[idx];
    }

    static constexpr auto read_at(self_t const& self, std::size_t idx) -> bool
    {
        return self[idx];
    }

    static constexpr auto move_at(self_t const& self, std::size_t idx) -> bool
    {
        return self[idx];
    }

    static constexpr auto inc(self_t const&, std::size_t& idx) -> std::size_t&
    {
        return ++idx;
    }

    static constexpr auto dec(self_t const&, std::size_t& idx) -> std::size_t&
    {
        return --idx;
    }

    static constexpr auto inc(self_t const&, std::size_t& idx, std::ptrdiff_t off)
        -> std::size_t&
    {
        return idx += static_cast<std::size_t>(off);
    }

    static constexpr auto distance(self_t const&, std::size_t from, std::size_t to)
        -> std::ptrdiff_t
    {
        return static_cast<std::ptrdiff_t>(to) - static_cast<std::ptrdiff_t>(from);
    }

    static constexpr auto last(self_t const&) -> std::size_t { return N; }

    static constexpr auto size(self_t const&) -> std::ptrdiff_t { return N; }

};


} // namespace flux

#endif



// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_GENERATOR_HPP_INCLUDED
#define FLUX_SOURCE_GENERATOR_HPP_INCLUDED



#include <coroutine>
#include <utility>

namespace flux {

FLUX_EXPORT
template <typename ElemT>
struct generator : inline_sequence_base<generator<ElemT>> {

    using yielded_type = std::conditional_t<std::is_reference_v<ElemT>,
                                            ElemT,
                                            ElemT const&>;

    struct promise_type;

    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        auto initial_suspend() { return std::suspend_always{}; }

        auto final_suspend() noexcept { return std::suspend_always{}; }

        auto get_return_object()
        {
            return generator(handle_type::from_promise(*this));
        }

        auto yield_value(yielded_type elem)
        {
            ptr_ = std::addressof(elem);
            return std::suspend_always{};
        }

        auto unhandled_exception() { throw; }

        void return_void() noexcept {}

        std::add_pointer_t<yielded_type> ptr_;
    };

private:
    handle_type coro_;

    explicit generator(handle_type&& handle) : coro_(std::move(handle)) {}

    friend struct sequence_traits<generator>;

public:
    generator(generator&& other) noexcept
        : coro_(std::exchange(other.coro_, {}))
    {}

    generator& operator=(generator&& other) noexcept
    {
        std::swap(coro_, other.coro_);
        return *this;
    }

    ~generator()
    {
        if (coro_) { coro_.destroy(); }
    }
};

template <typename T>
struct sequence_traits<generator<T>> : default_sequence_traits
{
private:
    struct cursor_type {
        cursor_type(cursor_type&&) = default;
        cursor_type& operator=(cursor_type&&) = default;
    private:
        cursor_type() = default;
        friend struct sequence_traits;
    };

    using self_t = generator<T>;

public:
    static auto first(self_t& self) {
        self.coro_.resume();
        return cursor_type{};
    }

    static auto is_last(self_t& self, cursor_type const&) -> bool
    {
        return self.coro_.done();
    }

    static auto inc(self_t& self, cursor_type& cur) -> cursor_type&
    {
        self.coro_.resume();
        return cur;
    }

    static auto read_at(self_t& self, cursor_type const&) -> decltype(auto)
    {
        return static_cast<typename self_t::yielded_type>(*self.coro_.promise().ptr_);
    }
};

} // namespace flux

#endif // FLUX_SOURCE_GENERATOR_HPP_INCLUDED

// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_GETLINES_HPP_INCLUDED
#define FLUX_SOURCE_GETLINES_HPP_INCLUDED



#include <iosfwd>
#include <string>

namespace flux {

namespace detail {

template <typename CharT, typename Traits>
struct getlines_sequence : inline_sequence_base<getlines_sequence<CharT, Traits>> {
private:
    using istream_type = std::basic_istream<CharT, Traits>;
    using string_type = std::basic_string<CharT, Traits>;
    using char_type = CharT;

    istream_type* is_ = nullptr;
    string_type str_;
    char_type delim_{};

public:
    getlines_sequence() = default;

    getlines_sequence(istream_type& is, char_type delim)
        : is_(std::addressof(is)),
          delim_(delim)
    {}

    getlines_sequence(getlines_sequence&&) = default;
    getlines_sequence& operator=(getlines_sequence&&) = default;

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            explicit cursor_type() = default;
            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;
        };

        using self_t = getlines_sequence;

    public:
        static constexpr auto first(self_t& self) -> cursor_type
        {
            cursor_type cur{};
            inc(self, cur);
            return cur;
        }

        static constexpr auto is_last(self_t& self, cursor_type const&) -> bool
        {
            return !(self.is_ && static_cast<bool>(*self.is_));
        }

        static constexpr auto inc(self_t& self, cursor_type& cur) -> cursor_type&
        {
            flux::assert_(self.is_ != nullptr,
                         "flux::getlines::inc(): attempt to iterate after stream EOF");
            if (!std::getline(*self.is_, self.str_, self.delim_)) {
                self.is_ = nullptr;
            }
            return cur;
        }

        static constexpr auto read_at(self_t& self, cursor_type const&) -> string_type const&
        {
            return self.str_;
        }
    };
};

struct getlines_fn {
    template <typename CharT, typename Traits>
    constexpr auto operator()(std::basic_istream<CharT, Traits>& istream, CharT delim) const
    {
        return getlines_sequence<CharT, Traits>(istream, delim);
    }

    template <typename CharT, typename Traits>
    constexpr auto operator()(std::basic_istream<CharT, Traits>& istream) const
    {
        return getlines_sequence<CharT, Traits>(istream, istream.widen('\n'));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto getlines = detail::getlines_fn{};

} // namespace flux

#endif // FLUX_SOURCE_GETLINES_HPP_INCLUDED



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
class istream_adaptor : public inline_sequence_base<istream_adaptor<T, CharT, Traits>> {
    using istream_type = std::basic_istream<CharT, Traits>;
    istream_type* is_ = nullptr;
    T val_ = T();

    friend struct sequence_traits<istream_adaptor>;

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
struct sequence_traits<detail::istream_adaptor<T, CharT, Traits>> : default_sequence_traits
{
private:
    struct cursor_type {
        cursor_type(cursor_type&&) = default;
        cursor_type& operator=(cursor_type&&) = default;
    private:
        friend struct sequence_traits;
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

FLUX_EXPORT
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
    auto operator()(std::basic_streambuf<CharT, Traits>* streambuf) const -> sequence auto
    {
        FLUX_ASSERT(streambuf != nullptr);
        return flux::mut_ref(*streambuf);
    }

    template <typename CharT, typename Traits>
    [[nodiscard]]
    auto operator()(std::basic_istream<CharT, Traits>& istream) const -> sequence auto
    {
        return flux::mut_ref(*istream.rdbuf());
    }
};

} // namespace detail

template <detail::derives_from_streambuf Streambuf>
struct sequence_traits<Streambuf> : default_sequence_traits
{
private:
    struct cursor_type {
        cursor_type(cursor_type&&) = default;
        cursor_type& operator=(cursor_type&&) = default;
    private:
        friend struct sequence_traits;
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

FLUX_EXPORT
inline constexpr auto from_istreambuf = detail::from_istreambuf_fn{};

} // namespace flux

#endif


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_RANGES_SOURCE_RANGE_HPP_INCLUDED
#define FLUX_RANGES_SOURCE_RANGE_HPP_INCLUDED



#include <ranges>

namespace flux {

namespace detail {

template <typename R>
concept can_const_iterate =
    std::ranges::input_range<R> && std::ranges::input_range<R const> &&
    std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<R const>>;

template <typename R, bool IsConst>
struct range_sequence : inline_sequence_base<range_sequence<R, IsConst>> {
private:
    R rng_;

    using V = std::conditional_t<IsConst, R const, R>;

public:
    struct flux_sequence_traits : default_sequence_traits {
    private:
        class cursor_type {

            std::ranges::iterator_t<V> iter;

            friend struct flux_sequence_traits;

            constexpr explicit cursor_type(std::ranges::iterator_t<V> iter) : iter(std::move(iter)) {}
        public:
            cursor_type() = default;
            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;
            cursor_type(cursor_type const&) requires std::ranges::forward_range<V>
                = default;
            cursor_type& operator=(cursor_type const&) requires std::ranges::forward_range<V>
                = default;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool = default;

            friend auto operator<=>(cursor_type const& lhs, cursor_type const& rhs)
                -> std::strong_ordering
                = default;

        };

    public:
        using value_type = std::ranges::range_value_t<R>;

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto first(Self& self) -> cursor_type
        {
            if constexpr (IsConst) {
                return cursor_type{std::ranges::cbegin(self.rng_)};
            } else {
                return cursor_type{std::ranges::begin(self.rng_)};
            }
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            if constexpr (IsConst) {
                return cur.iter == std::ranges::cend(self.rng_);
            } else {
                return cur.iter == std::ranges::end(self.rng_);
            }
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto inc(Self& self, cursor_type& cur)
        {
            bounds_check(!is_last(self, cur));
            ++cur.iter;
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto read_at(Self& self, cursor_type const& cur) -> decltype(auto)
        {
            bounds_check(!is_last(self, cur));
            return *cur.iter;
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto move_at(Self& self, cursor_type const& cur) -> decltype(auto)
        {
            bounds_check(!is_last(self, cur));
            return std::ranges::iter_move(cur.iter);
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto dec(Self& self, cursor_type& cur)
            requires std::ranges::bidirectional_range<R>
        {
            bounds_check(cur != first(self));
            --cur.iter;
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto inc(Self& self, cursor_type& cur, distance_t offset)
            requires std::ranges::random_access_range<R>
        {
            if (offset < 0) {
                bounds_check(num::checked_add(offset, distance(self, first(self), cur)) >= 0);
            } else if (offset > 0) {
                bounds_check(offset < distance(self, cur, last(self)));
            }

            cur.iter += checked_cast<std::ranges::range_difference_t<V>>(offset);
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto distance(Self&, cursor_type const& from, cursor_type const& to)
            -> distance_t
            requires std::ranges::random_access_range<R>
        {
            return checked_cast<distance_t>(std::ranges::distance(from.iter, to.iter));
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto data(Self& self)
            requires std::ranges::contiguous_range<R>
        {
            if constexpr (IsConst) {
                return std::ranges::cdata(self.rng_);
            } else {
                return std::ranges::data(self.rng_);
            }
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto last(Self& self) -> cursor_type
            requires std::ranges::common_range<R>
        {
            return cursor_type{std::ranges::end(self.rng_)};
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto size(Self& self) -> distance_t
            requires std::ranges::sized_range<R>
        {
            return checked_cast<distance_t>(std::ranges::ssize(self.rng_));
        }

        template <typename Self>
            requires (!std::is_const_v<Self> || can_const_iterate<V>)
        static constexpr auto for_each_while(Self& self, auto&& pred) -> cursor_type
        {
            auto iter = std::ranges::begin(self.rng_);
            auto const end = std::ranges::end(self.rng_);

            while (iter != end) {
                if (!std::invoke(pred, *iter)) {
                    break;
                }
                ++iter;
            }
            return cursor_type{std::move(iter)};
        }
    };

    constexpr explicit range_sequence(R rng) : rng_(std::move(rng)) {}

    constexpr auto begin()
    {
        if constexpr (IsConst) {
            return std::ranges::cbegin(rng_);
        } else {
            return std::ranges::begin(rng_);
        }
    }

    constexpr auto begin() const requires std::ranges::range<R const>
    {
        return std::ranges::begin(rng_);
    }

    constexpr auto end()
    {
        if constexpr (IsConst) {
            return std::ranges::cend(rng_);
        } else {
            return std::ranges::end(rng_);
        }
    }

    constexpr auto end() const requires std::ranges::range<R const>
    {
        return std::ranges::end(rng_);
    }
};

struct from_range_fn {
    template <std::ranges::viewable_range R>
    requires std::ranges::input_range<R>
    constexpr auto operator()(R&& rng) const
    {
        return range_sequence<std::views::all_t<R>, false>(std::views::all(FLUX_FWD(rng)));
    }
};

struct from_crange_fn {
    template <typename R, typename C = std::remove_reference_t<R> const&>
        requires std::ranges::input_range<C> &&
                 std::ranges::viewable_range<C>
    constexpr auto operator()(R&& rng) const
    {
        if constexpr (std::is_lvalue_reference_v<R>) {
            return range_sequence<std::views::all_t<C>, true>(std::views::all(static_cast<C>(rng)));
        } else {
            return range_sequence<std::views::all_t<R>, true>(std::views::all(FLUX_FWD(rng)));
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto from_range = detail::from_range_fn{};
FLUX_EXPORT inline constexpr auto from_crange = detail::from_crange_fn{};

} // namespace flux

#endif


// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_REPEAT_HPP_INCLUDED
#define FLUX_SOURCE_REPEAT_HPP_INCLUDED



namespace flux {

namespace detail {

template <bool>
struct repeat_data {};

template <>
struct repeat_data<false> { std::size_t count; };

template <std::movable T, bool IsInfinite>
struct repeat_sequence : inline_sequence_base<repeat_sequence<T, IsInfinite>>
{
private:
    T obj_;
    FLUX_NO_UNIQUE_ADDRESS repeat_data<IsInfinite> data_;

public:
    constexpr explicit repeat_sequence(decays_to<T> auto&& obj)
        requires IsInfinite
        : obj_(FLUX_FWD(obj))
    {}

    constexpr repeat_sequence(decays_to<T> auto&& obj, std::size_t count)
        requires (!IsInfinite)
        : obj_(FLUX_FWD(obj)),
          data_{count}
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        using self_t = repeat_sequence;

    public:
        static inline constexpr bool is_infinite = IsInfinite;

        static constexpr auto first(self_t const&) -> std::size_t { return 0; }

        static constexpr auto is_last(self_t const& self, std::size_t cur) -> bool
        {
            if constexpr (IsInfinite) {
                return false;
            } else {
                return cur >= self.data_.count;
            }
        }

        static constexpr auto inc(self_t const&, std::size_t& cur) -> void
        {
            ++cur;
        }

        static constexpr auto read_at(self_t const& self, std::size_t) -> T const&
        {
            return self.obj_;
        }

        static constexpr auto dec(self_t const&, std::size_t& cur) -> void
        {
            --cur;
        }

        static constexpr auto inc(self_t const&, std::size_t& cur, distance_t offset) -> void
        {
            cur += static_cast<std::size_t>(offset);
        }

        static constexpr auto distance(self_t const&, std::size_t from, std::size_t to) -> distance_t
        {
            return checked_cast<distance_t>(to) - checked_cast<distance_t>(from);
        }

        static constexpr auto for_each_while(self_t const& self, auto&& pred) -> std::size_t
        {
            if constexpr (IsInfinite) {
                std::size_t idx = 0;
                while (true) {
                    if (!std::invoke(pred, std::as_const(self.obj_))) {
                        return idx;
                    }
                    ++idx;
                }
            } else {
                std::size_t idx = 0;
                for ( ; idx < self.data_.count; ++idx) {
                    if (!std::invoke(pred, std::as_const(self.obj_))) {
                        break;
                    }
                }
                return idx;
            }
        }

        static constexpr auto last(self_t const& self) -> std::size_t
            requires (!IsInfinite)
        {
            return self.data_.count;
        }

        static constexpr auto size(self_t const& self) -> distance_t
            requires (!IsInfinite)
        {
            return checked_cast<distance_t>(self.data_.count);
        }
    };
};

struct repeat_fn {
    template <typename T>
        requires std::movable<std::decay_t<T>>
    constexpr auto operator()(T&& obj) const
    {
        return repeat_sequence<std::decay_t<T>, true>(FLUX_FWD(obj));
    }

    template <typename T>
        requires std::movable<std::decay_t<T>>
    constexpr auto operator()(T&& obj, std::integral auto count) const
    {
        auto c = checked_cast<distance_t>(count);
        if (c < 0) {
            runtime_error("Negative count passed to repeat()");
        }
        return repeat_sequence<std::decay_t<T>, false>(
            FLUX_FWD(obj), checked_cast<std::size_t>(c));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto repeat = detail::repeat_fn{};

} // namespace flux

#endif



// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_UNFOLD_HPP_INCLUDED
#define FLUX_SOURCE_UNFOLD_HPP_INCLUDED



namespace flux {

namespace detail {

template <typename R, typename Func>
struct unfold_sequence : inline_sequence_base<unfold_sequence<R, Func>> {
private:
    R state_;
    Func func_;

public:
    template <typename T>
        requires std::constructible_from<R, T>
    constexpr explicit unfold_sequence(Func&& func, T&& seed)
        : state_(FLUX_FWD(seed)),
          func_(std::move(func))
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            friend struct flux_sequence_traits;
            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;
        private:
            cursor_type() = default;
        };

        using self_t = unfold_sequence;

    public:
        static constexpr bool is_infinite = true;

        static constexpr auto first(self_t&) -> cursor_type { return {}; }

        static constexpr auto is_last(self_t&, cursor_type const&) -> bool { return false; }

        static constexpr auto inc(self_t& self, cursor_type&) -> void
        {
            self.state_ = std::invoke(self.func_, std::move(self.state_));
        }

        static constexpr auto read_at(self_t& self, cursor_type const&) -> R const&
        {
            return self.state_;
        }

        static constexpr auto for_each_while(self_t& self, auto&& pred) -> cursor_type
        {
            while (true) {
                if (!std::invoke(pred, self.state_)) {
                    break;
                }
                self.state_ = std::invoke(self.func_, std::move(self.state_));
            }

            return {};
        }
    };
};

struct unfold_fn {
    template <typename Func, typename Seed,
              typename R = std::decay_t<std::invoke_result_t<Func&, Seed>>>
        requires std::constructible_from<R, Seed> &&
                 std::invocable<Func&, R> &&
                 std::assignable_from<R&, std::invoke_result_t<Func&, R>>
    [[nodiscard]]
    constexpr auto operator()(Func func, Seed&& seed) const -> sequence auto
    {
        return unfold_sequence<R, Func>(std::move(func), FLUX_FWD(seed));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto unfold = detail::unfold_fn{};

} // namespace flux

#endif // FLUX_SOURCE_UNFOLD_INCLUDED


#endif
