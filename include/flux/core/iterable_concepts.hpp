// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_ITERABLE_CONCEPTS_HPP_INCLUDED
#define FLUX_CORE_ITERABLE_CONCEPTS_HPP_INCLUDED

#include <flux/core/concepts.hpp>
#include <flux/core/numeric.hpp>
#include <flux/core/optional.hpp>

namespace flux {

/*
 * MARK: Iteration context
 */

FLUX_EXPORT
enum class iteration_result : bool { incomplete = false, complete = true };

FLUX_EXPORT inline constexpr bool loop_break = false;
FLUX_EXPORT inline constexpr bool loop_continue = true;

FLUX_EXPORT template <typename Ctx>
using context_element_t = typename std::remove_cvref_t<Ctx>::element_type;

FLUX_EXPORT template <typename Ctx>
concept iteration_context
    = requires { typename Ctx::element_type; } && detail::can_reference<typename Ctx::element_type>
    && requires(Ctx& ctx, bool (*pred)(context_element_t<Ctx>)) {
           { ctx.run_while(flux::copy(pred)) } -> std::same_as<iteration_result>;
       };

FLUX_EXPORT
struct run_while_t {
    template <iteration_context Ctx, typename Pred>
        requires callable_mut<Pred, bool(context_element_t<Ctx>)>
    constexpr auto operator()(Ctx& ctx, Pred&& pred) const -> iteration_result
    {
        return ctx.run_while(FLUX_FWD(pred));
    }
};

FLUX_EXPORT inline constexpr run_while_t run_while {};

FLUX_EXPORT
struct step_t {
    template <iteration_context Ctx, typename Fn>
        requires callable_once<Fn, void(context_element_t<Ctx>)>
    constexpr auto operator()(Ctx& ctx, Fn fn) const
    {
        using E = context_element_t<Ctx>;
        using R = callable_result_t<Fn, E>;

        if constexpr (std::is_void_v<R>) {
            bool called = false;
            run_while(ctx, [&](auto&& elem) {
                std::move(fn)(FLUX_FWD(elem));
                called = true;
                return loop_break;
            });
            return called;
        } else {
            // If element_type is an rvalue reference, call with a value instead
            using T = std::conditional_t<std::is_rvalue_reference_v<E>, std::remove_cvref_t<E>, E>;

            flux::optional<T> opt = nullopt;
            run_while(ctx, [&](auto&& elem) {
                opt.emplace(std::move(fn)(FLUX_FWD(elem)));
                return loop_break;
            });
            return opt;
        }
    }
};

FLUX_EXPORT inline constexpr step_t step{};

FLUX_EXPORT
struct next_element_t {
    template <iteration_context Ctx>
    constexpr auto operator()(Ctx& ctx) const
        -> std::conditional_t<std::is_rvalue_reference_v<context_element_t<Ctx>>,
                              flux::optional<std::remove_cvref_t<context_element_t<Ctx>>>,
                              flux::optional<context_element_t<Ctx>>>
    {
        return step(ctx, std::identity{});
    }
};

FLUX_EXPORT inline constexpr next_element_t next_element{};

/*
 * MARK: Iterable
 */

FLUX_EXPORT template <typename>
struct iterable_traits {
    using _flux_is_primary_template = std::true_type;
};

namespace detail {

template <typename T>
concept has_member_iterable_traits = requires { typename T::flux_iterable_traits; }
    && std::is_class_v<typename T::flux_iterable_traits>;

} // namespace detail

template <typename T>
    requires detail::has_member_iterable_traits<T>
struct iterable_traits<T> : T::flux_iterable_traits { };

namespace detail {

template <typename It>
using iter_traits_t = iterable_traits<std::remove_cvref_t<It>>;

template <typename T>
concept has_iter_traits = !requires { typename iter_traits_t<T>::_flux_is_primary_template; };

template <typename T>
concept has_valid_iter_traits = has_iter_traits<T> && requires(T& t) {
    { iter_traits_t<T>::iterate(t) } -> iteration_context;
};

template <typename T>
concept has_member_iterate = requires(T& t) {
    { t.iterate() } -> iteration_context;
};

template <typename T>
concept can_iterate = has_valid_iter_traits<T> || has_member_iterate<T> || sequence<T>
    || std::ranges::input_range<T>;

template <std::ranges::input_range R>
struct range_iteration_context : immovable {
private:
    std::ranges::iterator_t<R> iter_;
    std::ranges::sentinel_t<R> sent_;
    bool increment_next_ = false;

public:
    using element_type = std::ranges::range_reference_t<R>;

    constexpr explicit range_iteration_context(R& rng)
        : iter_(std::ranges::begin(rng)),
          sent_(std::ranges::end(rng))
    {
    }

    constexpr auto run_while(auto&& pred) -> iteration_result
    {
        if (increment_next_ && iter_ != sent_) {
            ++iter_;
            increment_next_ = false;
        }

        while (iter_ != sent_) {
            if (!pred(*iter_)) {
                increment_next_ = true;
                return iteration_result::incomplete;
            }
            ++iter_;
        }

        return iteration_result::complete;
    }
};

template <std::ranges::forward_range R>
struct range_iteration_context<R> : immovable {
private:
    std::ranges::iterator_t<R> iter_;
    std::ranges::sentinel_t<R> sent_;

public:
    using element_type = std::ranges::range_reference_t<R>;

    constexpr explicit range_iteration_context(R& rng)
        : iter_(std::ranges::begin(rng)),
          sent_(std::ranges::end(rng))
    {
    }

    constexpr auto run_while(auto&& pred) -> iteration_result
    {
        while (iter_ != sent_) {
            if (!pred(*iter_++)) {
                return iteration_result::incomplete;
            }
        }

        return iteration_result::complete;
    }
};

template <sequence Seq>
struct sequence_iteration_context : immovable {
private:
    Seq* ptr_;
    cursor_t<Seq> cur_;
    bool inc_next_ = false;

public:
    using element_type = element_t<Seq>;

    constexpr explicit sequence_iteration_context(Seq& seq)
        : ptr_(std::addressof(seq)),
          cur_(first(seq))
    {
    }

    constexpr auto run_while(auto&& pred) -> iteration_result
    {
        if (inc_next_ && !is_last(*ptr_, cur_)) {
            inc(*ptr_, cur_);
            inc_next_ = false;
        }

        while (!is_last(*ptr_, cur_)) {
            if (!pred(read_at_unchecked(*ptr_, cur_))) {
                inc_next_ = true;
                return iteration_result::incomplete;
            }
            inc(*ptr_, cur_);
        }

        return iteration_result::complete;
    }
};

template <multipass_sequence Seq>
struct sequence_iteration_context<Seq> {
private:
    Seq* ptr_;
    cursor_t<Seq> cur_;

public:
    using element_type = element_t<Seq>;

    constexpr explicit sequence_iteration_context(Seq& seq)
        : ptr_(std::addressof(seq)),
          cur_(first(seq))
    {
    }

    constexpr auto run_while(auto&& pred) -> iteration_result
    {
        while (!is_last(*ptr_, cur_)) {
            auto _ = detail::defer_t([&] { inc(*ptr_, cur_); });

            if (!pred(read_at_unchecked(*ptr_, cur_))) {
                return iteration_result::incomplete;
            }
        }

        return iteration_result::complete;
    }
};

} // namespace detail

FLUX_EXPORT
struct iterate_t {
    template <typename It>
        requires detail::can_iterate<It>
    constexpr auto operator()(It& it) const -> iteration_context auto
    {
        if constexpr (detail::has_valid_iter_traits<It>) {
            return detail::iter_traits_t<It>::iterate(it);
        } else if constexpr (detail::has_member_iterate<It>) {
            return it.iterate();
        } else if constexpr (sequence<It>) {
            return detail::sequence_iteration_context<It>(it);
        } else if constexpr (std::ranges::input_range<It>) {
            return detail::range_iteration_context<It>(it);
        }
    }
};

FLUX_EXPORT inline constexpr iterate_t iterate {};

FLUX_EXPORT template <typename I>
using iteration_context_t = decltype(iterate(std::declval<I&>()));

FLUX_EXPORT template <typename I>
using iterable_element_t = context_element_t<iteration_context_t<I>>;

namespace detail {

// Try to work out the value type of the iterable
// * If iterable_context_t<T>::value_type exists, use that
// * otherwise, if iterable_traits<T>::value_type exists, use that
// * otherwise, if T::value_type exists, use that,
// * otherwise, fall back to std::remove_cvref_t<iterable_element_t<T>>

template <typename T>
concept has_context_value_type = requires { typename iteration_context_t<T>::value_type; };

template <typename T>
concept has_traits_value_type = requires { typename iter_traits_t<T>::value_type; };

template <typename T>
concept has_member_value_type = requires { typename T::value_type; };

template <typename T>
struct iterable_value_type {
    using type = std::remove_cvref_t<iterable_element_t<T>>;
};

template <typename T>
    requires has_context_value_type<T>
struct iterable_value_type<T> {
    using type = typename iteration_context_t<T>::value_type;
};

template <typename T>
    requires has_traits_value_type<T> && (!has_context_value_type<T>)
struct iterable_value_type<T> {
    using type = typename iter_traits_t<T>::value_type;
};

template <typename T>
    requires has_member_value_type<T> && (!has_context_value_type<T> && !has_traits_value_type<T>)
struct iterable_value_type<T> {
    using type = std::remove_cvref_t<iterable_element_t<T>>;
};

} // namespace detail

FLUX_EXPORT template <typename I>
using iterable_value_t = typename detail::iterable_value_type<I>::type;

FLUX_EXPORT template <typename Seq>
using iterable_common_element_t = std::common_reference_t<element_t<Seq>, value_t<Seq>&>;

FLUX_EXPORT template <typename It>
concept iterable = requires(It& it) {
    { iterate(it) } -> iteration_context;
} && requires {
    typename iterable_value_t<It>;
} && std::is_object_v<iterable_value_t<It>> && std::common_reference_with<iterable_element_t<It>&&, iterable_value_t<It>&>;

/*
 * MARK: Sized iterable
 */

namespace detail {

template <typename T>
concept has_iterable_traits_size = requires(T& t) {
    { iter_traits_t<T>::size(t) } -> num::integral;
};

template <typename T>
concept has_member_size = has_member_iterate<T> && requires(T& t) {
    { t.size() } -> num::integral;
};

template <typename T>
concept is_sized_iterable = has_iterable_traits_size<T> || has_member_size<T> || sized_sequence<T>
    || std::ranges::sized_range<T>;

} // namespace detail

struct iterable_size_fn_t {
    template <typename It>
        requires detail::is_sized_iterable<It>
    [[nodiscard]]
    constexpr auto operator()(It&& it) const -> int_t
    {
        if constexpr (detail::has_iterable_traits_size<It>) {
            return num::cast<int_t>(detail::iter_traits_t<It>::size(it));
        } else if constexpr (detail::has_member_size<It>) {
            return num::cast<int_t>(it.size());
        } else if constexpr (sized_sequence<It>) {
            return sequence_traits<std::remove_cvref_t<It>>::size(it);
        } else if constexpr (std::ranges::sized_range<It>) {
            return num::cast<int_t>(std::ranges::ssize(it));
        }
    }
};

FLUX_EXPORT inline constexpr iterable_size_fn_t iterable_size{};

FLUX_EXPORT template <typename It>
concept sized_iterable = iterable<It> && requires(It&& it) {
    { iterable_size(it) } -> std::same_as<int_t>;
};

/*
 * MARK: Reverse iterable
 */

namespace detail {

template <typename T>
concept has_reverse_iter_traits = requires(T& t) {
    { iter_traits_t<T>::reverse_iterate(t) } -> iteration_context;
};

template <typename T>
concept has_member_reverse_iterate = has_member_iterate<T> && requires(T& t) {
    { t.reverse_iterate() } -> iteration_context;
};

template <typename T>
concept can_reverse_iterate = has_reverse_iter_traits<T> || has_member_reverse_iterate<T>
    || (bidirectional_sequence<T> && bounded_sequence<T>)
    || (std::ranges::bidirectional_range<T> && std::ranges::common_range<T>);

template <std::ranges::bidirectional_range R>
    requires std::ranges::common_range<R>
struct range_reverse_iteration_context : immovable {
private:
    std::ranges::iterator_t<R> iter_;
    std::ranges::iterator_t<R> start_;

public:
    using element_type = std::ranges::range_reference_t<R>;

    constexpr explicit range_reverse_iteration_context(R& rng)
        : iter_(std::ranges::end(rng)),
          start_(std::ranges::begin(rng))
    {
    }

    constexpr auto run_while(auto&& pred) -> iteration_result
    {
        while (iter_ != start_) {
            if (!pred(*--iter_)) {
                return iteration_result::incomplete;
            }
        }

        return iteration_result::complete;
    }
};

template <bidirectional_sequence Seq>
    requires bounded_sequence<Seq>
struct sequence_reverse_iteration_context : immovable {
private:
    Seq* ptr_;
    cursor_t<Seq> cur_;
    cursor_t<Seq> start_;

public:
    using element_type = element_t<Seq>;

    constexpr explicit sequence_reverse_iteration_context(Seq& seq)
        : ptr_(std::addressof(seq)),
          cur_(last(seq)),
          start_(first(seq))
    {
    }

    constexpr auto run_while(auto&& pred) -> iteration_result
    {
        while (cur_ != start_) {
            dec(*ptr_, cur_);
            if (!pred(read_at_unchecked(*ptr_, cur_))) {
                return iteration_result::incomplete;
            }
        }

        return iteration_result::complete;
    }
};
} // namespace detail

FLUX_EXPORT struct reverse_iterate_t {
    template <typename It>
        requires detail::can_reverse_iterate<It>
    constexpr auto operator()(It& it) const -> iteration_context auto
    {
        if constexpr (detail::has_reverse_iter_traits<It>) {
            return iterable_traits<It>::reverse_iterate(it);
        } else if constexpr (detail::has_member_reverse_iterate<It>) {
            return it.reverse_iterate();
        } else if constexpr (bidirectional_sequence<It> && bounded_sequence<It>) {
            return detail::sequence_reverse_iteration_context<It>(it);
        } else if constexpr (std::ranges::bidirectional_range<It>
                             && std::ranges::common_range<It>) {
            return detail::range_reverse_iteration_context<It>(it);
        }
    }
};

FLUX_EXPORT inline constexpr reverse_iterate_t reverse_iterate {};

FLUX_EXPORT template <typename It>
concept reverse_iterable = iterable<It> && requires(It& it) {
    { reverse_iterate(it) } -> iteration_context;
};

FLUX_EXPORT template <reverse_iterable I>
using reverse_iteration_context_t = decltype(reverse_iterate(std::declval<I&>()));

} // namespace flux

#endif // FLUX_CORE_ITERABLE_CONCEPTS_HPP_INCLUDED