
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>

#include "test_utils.hpp"

namespace {

struct S {
    int i;

    constexpr int get() const { return i; }
};

constexpr bool test_starts_with()
{
    // Basic starts_with
    {
        std::array const arr1{1, 2, 3, 4, 5};
        int arr2[] = {1, 2, 3};

        STATIC_CHECK(flux::starts_with(arr1, arr2));
    }

    // Basic member starts_with
    {
        std::array const arr1{1, 2, 3, 4, 5};
        int arr2[] = {1, 2, 3};

        STATIC_CHECK(flux::ref(arr1).starts_with(arr2));
    }

    // Basic starts_with, failing
    {
        std::array const arr1{1, 2, 3, 4, 5};
        int arr2[] = {1, 2, 99};

        STATIC_CHECK(not flux::starts_with(arr1, arr2));
    }

    // Basic member starts_with, failing
    {
        std::array const arr1{1, 2, 3, 4, 5};
        int arr2[] = {1, 2, 99};

        STATIC_CHECK(not flux::ref(arr1).starts_with(arr2));
    }

    // A sequence always starts_with itself
    {
        std::array const arr{1, 2, 3, 4, 5};

        STATIC_CHECK(flux::starts_with(arr, arr));

        // ...even if they're both empty
        STATIC_CHECK(flux::starts_with(flux::empty<int>, flux::empty<int>));
    }

    // Needle is longer than haystack
    {
        std::array const arr1{1, 2, 3};
        int arr2[] = {1, 2, 3, 4, 5};

        STATIC_CHECK(not flux::starts_with(arr1, arr2));
    }

    // Cross-type starts_with using default comparator
    {
        std::array const arr1{1, 2, 3, 4, 5};
        double arr2[] = {1.0, 2.0, 3.0, 4.0, 99.0};

        STATIC_CHECK(flux::starts_with(arr1, flux::ref(arr2).take(3)));
        STATIC_CHECK(not flux::starts_with(arr1, arr2));
    }

    // starts_with using custom comparator
    {
        std::array const arr1{1, 2, 3, 4, 5};
        S arr2[] = {{1}, {2}, {3}};

        auto cmp = [](int i, S s) { return i == s.i; };

        STATIC_CHECK(flux::starts_with(arr1, arr2, cmp));
        STATIC_CHECK(flux::ref(arr1).starts_with(arr2, cmp));
    }

    // starts_with using projection
    {
        S arr1[] = {{1}, {2}, {3}, {4}, {5}};
        std::array const arr2{1, 2, 3};

        STATIC_CHECK(flux::starts_with(arr1, arr2, flux::proj2{std::equal_to{}, &S::i}));

        STATIC_CHECK(flux::ref(arr1).starts_with(arr2, flux::proj2{std::equal_to{}, &S::get}));

        STATIC_CHECK(flux::starts_with(arr1, arr1, flux::proj(std::equal_to{}, &S::i)));
    }

    return true;
}
static_assert(test_starts_with());

}

TEST_CASE("starts_with")
{
    bool result = test_starts_with();
    REQUIRE(result);
}