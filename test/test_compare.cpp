
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>

namespace {

struct Test {
    int i;

    constexpr bool operator==(Test other) const { return i == other.i; }
    constexpr bool operator<(Test other) const { return i < other.i; }
};

constexpr bool test_compare()
{
    // equal
    {
        std::array arr1{1, 2, 3, 4, 5};
        int arr2[] = {1, 2, 3, 4, 5};

        auto r = flux::compare(arr1, arr2);
        static_assert(std::same_as<decltype(r), std::strong_ordering>);
        STATIC_CHECK(r == 0);
    }

    // less
    {
        std::array arr1{1, 2, 3, 4, 0};
        int arr2[] = {1, 2, 3, 4, 5};

        auto r = flux::compare(arr1, arr2);
        static_assert(std::same_as<decltype(r), std::strong_ordering>);
        STATIC_CHECK(r == std::strong_ordering::less);
    }

    // greater
    {
        std::array arr1{1., 2., 3., 4., 5.};
        int arr2[] = {1, 2, 3, 4, 0};

        auto r = flux::compare(arr1, arr2);
        static_assert(std::same_as<decltype(r), std::partial_ordering>);
        STATIC_CHECK(std::is_gt(r));
    }

    // LHS has fewer elements => less
    {
        std::string_view s1 = "abcd";
        std::string_view s2 = "abcde";

        auto r = flux::compare(s1, s2);

        static_assert(std::same_as<decltype(r), std::strong_ordering>);
        STATIC_CHECK(r < 0);
    }

    // RHS has fewer elements => greater
    {
        std::string_view s1 = "abcde";
        std::string_view s2 = "abcd";

        auto r = flux::compare(s1, s2);

        static_assert(std::same_as<decltype(r), std::strong_ordering>);
        STATIC_CHECK(r > 0);
    }

    // empty sequences are equal
    {
        // ...but still require the element types to be comparable
        static_assert(not std::invocable<flux::detail::compare_fn, flux::detail::empty_sequence<int>, flux::detail::empty_sequence<std::nullptr_t>>);

        auto r = flux::compare(flux::empty<int>, flux::empty<int>);
        static_assert(std::same_as<decltype(r), std::strong_ordering>);
        STATIC_CHECK(r == std::strong_ordering::equal);
    }

    // can use custom comparator
    {
        std::array<Test, 3> arr1{Test{1}, {2}, {3}};
        Test arr2[] = {{1}, {2}, {3}};

        auto r = flux::compare(arr1, arr2, [](Test lhs, Test rhs) {
            return lhs.i <=> rhs.i;
        });
        static_assert(std::same_as<decltype(r), std::strong_ordering>);
        STATIC_CHECK(r == std::strong_ordering::equal);
    }

    // Can use stdlib fallback fns for types with == and <
    {
        std::array<Test, 3> arr1{Test{1}, {2}, {3}};
        Test arr2[] = {{1}, {2}, {3}};

        auto r = flux::compare(arr1, arr2, std::compare_weak_order_fallback);
        static_assert(std::same_as<decltype(r), std::weak_ordering>);
        STATIC_CHECK(r == std::weak_ordering::equivalent);
    }

    // Can use projections
    {
        std::array<Test, 3> arr1{Test{1}, {2}, {3}};
        Test arr2[] = {{1}, {2}, {3}};

        auto r = flux::compare(arr1, arr2,
                               flux::proj2(std::compare_three_way{}, &Test::i, [](Test t) { return t.i; }));
        static_assert(std::same_as<decltype(r), std::strong_ordering>);
        STATIC_CHECK(r == std::strong_ordering::equal);
    }

    {
        std::array<std::uint8_t, 3> arr1{1, 2, 3};
        std::array<std::uint8_t, 3> arr2{1, 2, 3};
        auto r = flux::compare(arr1, arr2);

        static_assert(std::same_as<decltype(r), std::strong_ordering>);
        STATIC_CHECK(r == std::strong_ordering::equal);
    }

    {
        std::array<std::uint8_t, 3> arr1{1, 2, 3};
        std::array<std::uint8_t, 0> arr2{};
        auto r = flux::compare(arr1, arr2);
        static_assert(std::same_as<decltype(r), std::strong_ordering>);
        STATIC_CHECK(r == std::strong_ordering::greater);
        
        auto r2 = flux::compare(arr2, arr1);
        STATIC_CHECK(r2 == std::strong_ordering::less);
    }

    {
        std::array<std::uint8_t, 3> arr1{1, 2, 3};
        std::array<std::uint8_t, 3> arr2{1, 2, 4};
        
        auto r1 = flux::compare(arr1, arr2);
        
        static_assert(std::same_as<decltype(r1), std::strong_ordering>);
        STATIC_CHECK(r1 == std::strong_ordering::less);
        
        auto r2 = flux::compare(arr2, arr1);
        STATIC_CHECK(r2 == std::strong_ordering::greater);
    }

    return true;
}
static_assert(test_compare());

}

TEST_CASE("compare")
{
    REQUIRE(test_compare());
}