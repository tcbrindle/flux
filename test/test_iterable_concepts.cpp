// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <doctest/doctest.h>

#ifdef USE_MODULES
import flux;
#else
#    include <flux.hpp>
#endif

/*
 * Iteration context concept tests
 */

namespace {

static_assert(not flux::iteration_context<void>);
static_assert(not flux::iteration_context<void*>);
static_assert(not flux::iteration_context<int>);
static_assert(not flux::iteration_context<int[]>);

struct empty { };
static_assert(not flux::iteration_context<empty>);

struct incomplete;
static_assert(not flux::iteration_context<incomplete>);

struct no_element_type {
    auto run_while(auto&&) -> flux::iteration_result;
};
static_assert(not flux::iteration_context<no_element_type>);

struct void_element_type {
    using element_type = void;
    auto run_while(auto&&) -> flux::iteration_result;
};
static_assert(not flux::iteration_context<void_element_type>);

struct no_run_while {
    using element_type = int;
};
static_assert(not flux::iteration_context<no_run_while>);

struct run_while_returns_void {
    using element_type = int;
    auto run_while(auto&&) -> void;
};
static_assert(not flux::iteration_context<run_while_returns_void>);

struct run_while_returns_int {
    using element_type = int;
    auto run_while(auto&&) -> int;
};
static_assert(not flux::iteration_context<run_while_returns_int>);

struct run_while_takes_void {
    using element_type = int;
    auto run_while() -> flux::iteration_result;
};
static_assert(not flux::iteration_context<run_while_takes_void>);

struct minimal_iteration_context {
    using element_type = int;
    auto run_while(auto&&) -> flux::iteration_result;
};
static_assert(flux::iteration_context<minimal_iteration_context>);

} // namespace

/*
 * Iterable concept tests
 */

namespace {

struct has_empty_iter_traits_specialisation { };
struct has_incorrect_iter_traits_specialisation { };
struct has_valid_iter_traits_specialisation { };
} // namespace

template <>
struct flux::iterable_traits<has_empty_iter_traits_specialisation> { };

template <>
struct flux::iterable_traits<has_incorrect_iter_traits_specialisation> {
    auto iterate() -> void;
};

template <>
struct flux::iterable_traits<has_valid_iter_traits_specialisation> {
    static auto iterate(has_valid_iter_traits_specialisation&) -> minimal_iteration_context
    {
        return {};
    }
};

namespace {

// Things that are not iterable

static_assert(not flux::iterable<int>);
static_assert(not flux::iterable<int*>);
static_assert(not flux::iterable<int[]>);
static_assert(not flux::iterable<void>);
static_assert(not flux::iterable<void*>);

static_assert(not flux::iterable<incomplete>);
static_assert(not flux::iterable<empty>);

static_assert(not flux::iterable<has_empty_iter_traits_specialisation>);
static_assert(not flux::iterable<has_incorrect_iter_traits_specialisation>);

struct member_iterate_returns_int {
    auto iterate() -> int;
};
static_assert(not flux::iterable<member_iterate_returns_int>);

// Things that *are* iterable

static_assert(flux::iterable<has_valid_iter_traits_specialisation>);

struct minimal_iterable {
    auto iterate() -> minimal_iteration_context { return {}; }
};
static_assert(flux::iterable<minimal_iterable>);

static_assert(flux::iterable<int (&)[5]>);
static_assert(flux::iterable<int const (&)[5]>);
static_assert(flux::iterable<std::initializer_list<int>>);

static_assert(flux::iterable<decltype(flux::iota(0, 10))>);

} // namespace

/*
 * Reverse iterable concept tests
 */

namespace {

struct has_reverse_iter_traits { };

} // namespace

template <>
struct flux::iterable_traits<has_reverse_iter_traits> {
    static auto iterate(has_reverse_iter_traits&) -> minimal_iteration_context { return {}; }

    static auto reverse_iterate(has_reverse_iter_traits&) -> minimal_iteration_context
    {
        return {};
    }
};

namespace {

// Things that are not reverse iterable
static_assert(not flux::reverse_iterable<int>);
static_assert(not flux::reverse_iterable<int*>);
static_assert(not flux::reverse_iterable<int[]>);
static_assert(not flux::reverse_iterable<void>);
static_assert(not flux::reverse_iterable<void*>);

static_assert(not flux::reverse_iterable<incomplete>);
static_assert(not flux::reverse_iterable<empty>);

static_assert(not flux::reverse_iterable<has_empty_iter_traits_specialisation>);
static_assert(not flux::reverse_iterable<has_incorrect_iter_traits_specialisation>);

// Things that *are* reverse iterable
static_assert(flux::reverse_iterable<has_reverse_iter_traits>);

struct has_member_reverse_iterate {
    auto iterate() -> minimal_iteration_context { return {}; }
    auto reverse_iterate() -> minimal_iteration_context { return {}; }
};

static_assert(flux::reverse_iterable<has_member_reverse_iterate>);

static_assert(flux::reverse_iterable<int (&)[5]>);
static_assert(flux::reverse_iterable<int const (&)[5]>);
static_assert(flux::reverse_iterable<std::initializer_list<int>>);

static_assert(flux::reverse_iterable<decltype(flux::iota(0, 10))>);

} // namespace

/*
 * sized_iterable concept tests
 */

namespace {

// Things that are not sized iterable

static_assert(not flux::sized_iterable<int>);
static_assert(not flux::sized_iterable<int*>);
static_assert(not flux::sized_iterable<int[]>);
static_assert(not flux::sized_iterable<void>);
static_assert(not flux::sized_iterable<void*>);
static_assert(not flux::sized_iterable<incomplete>);
static_assert(not flux::sized_iterable<empty>);
static_assert(not flux::sized_iterable<has_empty_iter_traits_specialisation>);
static_assert(not flux::sized_iterable<has_incorrect_iter_traits_specialisation>);
static_assert(not flux::sized_iterable<has_valid_iter_traits_specialisation>);

struct has_invalid_member_size {
    auto iterate() -> minimal_iteration_context { return {}; }
    auto size() -> bool { return false; }
};

static_assert(not flux::sized_iterable<has_invalid_member_size>);

struct has_sized_iter_traits_specialisation { };

} // namespace

template <>
struct flux::iterable_traits<has_sized_iter_traits_specialisation> {
    static auto iterate(has_sized_iter_traits_specialisation&) -> minimal_iteration_context
    {
        return {};
    }

    static auto size(has_sized_iter_traits_specialisation&) -> int_t { return 0; }
};

namespace {

// Things that *are* sized iterable
static_assert(flux::sized_iterable<has_sized_iter_traits_specialisation>);
static_assert(flux::sized_iterable<int (&)[5]>);
static_assert(flux::sized_iterable<int const (&)[5]>);
static_assert(flux::sized_iterable<std::initializer_list<int>>);

struct has_member_size {
    auto iterate() -> minimal_iteration_context { return {}; }
    auto size() -> int { return 0; }
};
static_assert(flux::sized_iterable<has_member_size>);

} // namespace
