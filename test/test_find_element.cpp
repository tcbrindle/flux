// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <doctest/doctest.h>

#include "test_utils.hpp"

namespace {

constexpr bool test_find_element_if()
{
    {
        std::array arr{1, 2, 3, 4, 5};

        auto elem = flux::find_element_if(arr, flux::pred::eq(3));

        static_assert(std::same_as<decltype(elem), flux::optional<int&>>);

        STATIC_CHECK(elem.has_value());
        STATIC_CHECK(*elem == 3);
        STATIC_CHECK(&*elem == arr.data() + 2);

        *elem = 99;
        STATIC_CHECK(check_equal(arr, {1, 2, 99, 4, 5}));
    }

    {
        std::array const arr{1, 2, 3, 4, 5};

        auto elem = flux::find_element_if(arr, [](auto&&) { return false; });

        static_assert(std::same_as<decltype(elem), flux::optional<int const&>>);

        STATIC_CHECK(not elem.has_value());
    }

    return true;
}
static_assert(test_find_element_if());

constexpr bool test_find_element()
{
    {
        std::array arr{1, 2, 3, 4, 5};

        auto elem = flux::find_element(arr, 3);

        static_assert(std::same_as<decltype(elem), flux::optional<int&>>);

        STATIC_CHECK(elem.has_value());
        STATIC_CHECK(*elem == 3);
        STATIC_CHECK(&*elem == arr.data() + 2);

        *elem = 99;
        STATIC_CHECK(check_equal(arr, {1, 2, 99, 4, 5}));
    }

    {
        std::array const arr{1, 2, 3, 4, 5};

        auto elem = flux::find_element(arr, 9999);

        static_assert(std::same_as<decltype(elem), flux::optional<int const&>>);

        STATIC_CHECK(not elem.has_value());
    }

    {
        std::array arr{1, 2, 3, 4, 5};

        auto copies = flux::map(arr, flux::copy);

        auto elem = flux::find_element(copies, 3);

        static_assert(std::same_as<decltype(elem), flux::optional<int>>);

        STATIC_CHECK(elem.has_value());
        STATIC_CHECK(*elem == 3);

        *elem = 999;

        STATIC_CHECK(check_equal(arr, {1, 2, 3, 4, 5}));
    }

    return true;
}
static_assert(test_find_element());

} // namespace

TEST_CASE("find_element")
{
    bool r = test_find_element_if();
    REQUIRE(r);

    r = test_find_element();
    REQUIRE(r);
}