// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <chrono>

#include "test_utils.hpp"

namespace {

struct incrementable_only {
    int i = 0;

    bool operator==(incrementable_only const&) const = default;
    constexpr auto& operator++()
    {
        ++i;
        return *this;
    }
    constexpr auto operator++(int)
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }
};

struct decrementable {
    int i = 0;

    bool operator==(decrementable const&) const = default;
    constexpr auto& operator++()
    {
        ++i;
        return *this;
    }
    constexpr auto operator++(int)
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }
    constexpr auto& operator--()
    {
        --i;
        return *this;
    }
    constexpr auto operator--(int)
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }
};

constexpr bool test_iota_unbounded()
{
    {
        auto f = flux::iota(incrementable_only{0});

        using F = decltype(f);

        static_assert(flux::iterable<F>);
        static_assert(not flux::sized_iterable<F>);
        static_assert(not flux::reverse_iterable<F>);
        static_assert(not flux::multipass_sequence<F>);
        static_assert(std::same_as<flux::iterable_element_t<F>, incrementable_only>);

        using I = incrementable_only;
        STATIC_CHECK(check_equal(flux::take(f, 5), {I{0}, I{1}, I{2}, I{3}, I{4}}));
    }

    {
        auto f = flux::iota(decrementable{0});

        using F = decltype(f);

        static_assert(flux::iterable<F>);
        static_assert(not flux::sized_iterable<F>);
        static_assert(not flux::reverse_iterable<F>); // Unbounded
        static_assert(not flux::multipass_sequence<F>);
        static_assert(std::same_as<flux::iterable_element_t<F>, decrementable>);

        using D = decrementable;
        STATIC_CHECK(check_equal(flux::take(f, 5), {D{0}, D{1}, D{2}, D{3}, D{4}}));
    }

    return true;
}
static_assert(test_iota_unbounded());

constexpr bool test_iota_bounded()
{
    {
        using I = incrementable_only;
        auto f = flux::iota(I{0}, I{5});

        using F = decltype(f);

        static_assert(flux::iterable<F>);
        static_assert(not flux::sized_iterable<F>);
        static_assert(not flux::reverse_iterable<F>);
        static_assert(not flux::multipass_sequence<F>);
        static_assert(std::same_as<flux::iterable_element_t<F>, I>);

        STATIC_CHECK(check_equal(f, {I{0}, I{1}, I{2}, I{3}, I{4}}));
    }

    {
        using D = decrementable;
        auto f = flux::iota(D{0}, D{5});

        using F = decltype(f);

        static_assert(flux::iterable<F>);
        static_assert(not flux::sized_iterable<F>);
        static_assert(flux::reverse_iterable<F>);
        static_assert(not flux::multipass_sequence<F>);
        static_assert(std::same_as<flux::iterable_element_t<F>, D>);

        STATIC_CHECK(check_equal(flux::reverse(f), {D{4}, D{3}, D{2}, D{1}, D{0}}));
    }

    return true;
}
static_assert(test_iota_bounded());

constexpr bool test_iota_sequence()
{
    {
        auto f = flux::ints();

        using F = decltype(f);

        static_assert(flux::iterable<F>);
        static_assert(flux::sized_iterable<F>);
        static_assert(flux::reverse_iterable<F>);
        static_assert(flux::random_access_sequence<F>);
        static_assert(std::same_as<flux::iterable_element_t<F>, flux::int_t>);

        STATIC_CHECK(check_equal(flux::take(f, 5), {0, 1, 2, 3, 4}));
    }

    {
        using namespace std::chrono_literals;

        auto f = flux::iota(0s, 5s);

        using F = decltype(f);

        static_assert(flux::iterable<F>);
        static_assert(not flux::sized_iterable<F>); // no difference_type
        static_assert(flux::reverse_iterable<F>);
        static_assert(not flux::random_access_sequence<F>); // no difference_type
        static_assert(std::same_as<flux::iterable_element_t<F>, std::chrono::seconds>);

        STATIC_CHECK(check_equal(flux::reverse(f), {4s, 3s, 2s, 1s, 0s}));
    }

    return true;
}
static_assert(test_iota_sequence());
}

TEST_CASE("iota")
{
    REQUIRE(test_iota_unbounded());
    REQUIRE(test_iota_bounded());
    REQUIRE(test_iota_sequence());
}
