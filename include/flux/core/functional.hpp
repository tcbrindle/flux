
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_FUNCTIONAL_HPP_INCLUDED
#define FLUX_CORE_FUNCTIONAL_HPP_INCLUDED

#include <flux/macros.hpp>

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

} // namespace detail

FLUX_EXPORT inline constexpr auto unpack = detail::unpack_fn{};

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

struct min_fn {
    template <typename T, typename U, typename Cmp = std::ranges::less>
        requires std::strict_weak_order<Cmp&, T&, U&>
    [[nodiscard]]
    constexpr auto operator()(T&& t, U&& u, Cmp cmp = Cmp{}) const
        -> std::common_reference_t<T, U>
    {
        return std::invoke(cmp, u, t) ? FLUX_FWD(u) : FLUX_FWD(t);
    }
};

struct max_fn {
    template <typename T, typename U, typename Cmp = std::ranges::less>
        requires std::strict_weak_order<Cmp&, T&, U&>
    [[nodiscard]]
    constexpr auto operator()(T&& t, U&& u, Cmp cmp = Cmp{}) const
        -> std::common_reference_t<T, U>
    {
        return !std::invoke(cmp, u, t) ? FLUX_FWD(u) : FLUX_FWD(t);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto min = detail::min_fn{};
FLUX_EXPORT inline constexpr auto max = detail::max_fn{};

} // namespace cmp

} // namespace flux

#endif
