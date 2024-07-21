
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>
#include <iostream>
#include <list>

#include "test_utils.hpp"

namespace {

constexpr bool test_zip_map()
{
    {
        int arr1[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        double arr2[] = {0, 100, 200, 300, 400};

        auto zipped = flux::zip_map(
            [](int first, double second) {
                return first + static_cast<int>(second);
            },
            flux::mut_ref(arr1), flux::mut_ref(arr2)
        );

        using Z = decltype(zipped);

        static_assert(flux::sequence<Z>);
        static_assert(flux::bidirectional_sequence<Z>);
        static_assert(flux::random_access_sequence<Z>);
        static_assert(flux::sized_sequence<Z>);
        static_assert(flux::bounded_sequence<Z>);

        static_assert(flux::sequence<Z const>);
        static_assert(flux::bidirectional_sequence<Z const>);
        static_assert(flux::random_access_sequence<Z const>);
        static_assert(flux::sized_sequence<Z const>);
        static_assert(flux::bounded_sequence<Z const>);

        static_assert(std::same_as<flux::element_t<Z>, int>);
        static_assert(std::same_as<flux::value_t<Z>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Z>, int>);

        // Believe it or not these are correct -- we've zipped two ref adaptors
        static_assert(std::same_as<flux::element_t<Z const>, int>);
        static_assert(std::same_as<flux::value_t<Z const>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Z const>, int>);


        STATIC_CHECK(flux::size(zipped) == 5);
        STATIC_CHECK(flux::is_last(zipped, flux::last(zipped)));

        {
            int n = 0;
            FLUX_FOR(auto p, zipped) {
                auto res = p;
                auto original_arr1_value = n;
                auto original_arr2_value = 100.0 * n;
                STATIC_CHECK(res == original_arr1_value + original_arr2_value);
                ++n;
            }
        }
    }

    {
        struct move_only {
            int i;
            constexpr move_only(int i) : i(i) {}
            move_only(move_only&&) = default;
            move_only& operator=(move_only&&) = default;
            constexpr bool operator==(int j) const { return i == j; }
        };

        move_only arr1[] = {1, 2, 3, 4, 5};
        move_only arr2[] = {100, 200, 300, 400, 500};

        auto zipped = flux::zip_map([](const move_only &a, const move_only &b) { return a.i + b.i; }, flux::ref(arr1),
                                    flux::ref(arr2));

        using Z = decltype(zipped);
        static_assert(flux::sequence<Z>);
        static_assert(flux::bidirectional_sequence<Z>);
        static_assert(flux::random_access_sequence<Z>);
        static_assert(flux::sized_sequence<Z>);
        static_assert(flux::bounded_sequence<Z>);

        static_assert(flux::sequence<Z const>);
        static_assert(flux::bidirectional_sequence<Z const>);
        static_assert(flux::random_access_sequence<Z const>);
        static_assert(flux::sized_sequence<Z const>);
        static_assert(flux::bounded_sequence<Z const>);

        static_assert(std::same_as<flux::element_t<Z>, int>);
        static_assert(std::same_as<flux::value_t<Z>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Z>, int>);

        // Believe it or not these are correct -- we've zipped two ref adaptors
        static_assert(std::same_as<flux::element_t<Z const>, int>);
        static_assert(std::same_as<flux::value_t<Z const>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Z const>, int>);

        {
            int n = 1;
            FLUX_FOR(auto p, zipped) {
                auto res = p;
                auto original_arr1_value = n;
                auto original_arr2_value = 100 * n;
                STATIC_CHECK(res == original_arr1_value + original_arr2_value);
                ++n;
            }
        }
    }

    {
        std::array<int, 0> arr1 = {};
        double arr2[] = {1.0, 2.0, 3.0};

        auto zipped = flux::zip_map(
            [](int first, double second) {
                return first + static_cast<int>(second);
            },
            flux::mut_ref(arr1), flux::mut_ref(arr2)
        );

        using Z = decltype(zipped);

        static_assert(flux::sequence<Z>);
        static_assert(flux::bidirectional_sequence<Z>);
        static_assert(flux::random_access_sequence<Z>);
        static_assert(flux::sized_sequence<Z>);
        static_assert(flux::bounded_sequence<Z>);

        static_assert(flux::sequence<Z const>);
        static_assert(flux::bidirectional_sequence<Z const>);
        static_assert(flux::random_access_sequence<Z const>);
        static_assert(flux::sized_sequence<Z const>);
        static_assert(flux::bounded_sequence<Z const>);

        static_assert(std::same_as<flux::element_t<Z>, int>);
        static_assert(std::same_as<flux::value_t<Z>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Z>, int>);

        // Believe it or not these are correct -- we've zipped two ref adaptors
        static_assert(std::same_as<flux::element_t<Z const>, int>);
        static_assert(std::same_as<flux::value_t<Z const>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Z const>, int>);


        STATIC_CHECK(flux::size(zipped) == 0);
        STATIC_CHECK(flux::is_last(zipped, flux::last(zipped)));
        STATIC_CHECK(flux::equal(flux::empty<int>, zipped));
    }
    {
        auto zipped = flux::zip_map(
            [](int const& first, int const& second) -> int const& {
                return first > second ? first : second;
            },
            std::array{0, 1, 2, 3, 4},
            std::array{0, 1, 2, 3, 4}
        );

        using Z = decltype(zipped);

        static_assert(flux::sequence<Z>);
        static_assert(flux::bidirectional_sequence<Z>);
        static_assert(flux::random_access_sequence<Z>);
        static_assert(flux::sized_sequence<Z>);
        static_assert(flux::bounded_sequence<Z>);

        static_assert(flux::sequence<Z const>);
        static_assert(flux::bidirectional_sequence<Z const>);
        static_assert(flux::random_access_sequence<Z const>);
        static_assert(flux::sized_sequence<Z const>);
        static_assert(flux::bounded_sequence<Z const>);

        static_assert(std::same_as<flux::element_t<Z>, int const&>);
        static_assert(std::same_as<flux::value_t<Z>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Z>, int const&&>);

        static_assert(std::same_as<flux::element_t<Z const>, int const&>);
        static_assert(std::same_as<flux::value_t<Z const>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Z const>, int const&&>);


        STATIC_CHECK(flux::size(zipped) == 5);
        STATIC_CHECK(flux::is_last(zipped, flux::last(zipped)));
    }
    {
        std::array<int, 0> arr1 = {};

        auto zipped = flux::zip_map(
            [](int first) {
                return first * 2;
            },
            flux::mut_ref(arr1)
        );

        using Z = decltype(zipped);

        static_assert(flux::sequence<Z>);
        static_assert(flux::bidirectional_sequence<Z>);
        static_assert(flux::random_access_sequence<Z>);
        static_assert(flux::sized_sequence<Z>);
        static_assert(flux::bounded_sequence<Z>);

        static_assert(flux::sequence<Z const>);
        static_assert(flux::bidirectional_sequence<Z const>);
        static_assert(flux::random_access_sequence<Z const>);
        static_assert(flux::sized_sequence<Z const>);
        static_assert(flux::bounded_sequence<Z const>);

        static_assert(std::same_as<flux::element_t<Z>, int>);
        static_assert(std::same_as<flux::value_t<Z>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Z>, int>);

        static_assert(std::same_as<flux::element_t<Z const>, int>);
        static_assert(std::same_as<flux::value_t<Z const>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Z const>, int>);


        STATIC_CHECK(flux::size(zipped) == 0);
        STATIC_CHECK(flux::is_last(zipped, flux::last(zipped)));
        STATIC_CHECK(flux::equal(flux::empty<int>, zipped));
    }
    {
        auto zipped = flux::zip_map(
            []() -> int {
                return 3;
            }
        );

        using Z = decltype(zipped);

        static_assert(flux::sequence<Z>);
        static_assert(flux::bidirectional_sequence<Z>);
        static_assert(flux::random_access_sequence<Z>);
        static_assert(flux::sized_sequence<Z>);
        static_assert(flux::bounded_sequence<Z>);

        static_assert(flux::sequence<Z const>);
        static_assert(flux::bidirectional_sequence<Z const>);
        static_assert(flux::random_access_sequence<Z const>);
        static_assert(flux::sized_sequence<Z const>);
        static_assert(flux::bounded_sequence<Z const>);

        static_assert(std::same_as<flux::element_t<Z>, int&>);
        static_assert(std::same_as<flux::value_t<Z>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Z>, int&&>);

        static_assert(std::same_as<flux::element_t<Z const>, int&>);
        static_assert(std::same_as<flux::value_t<Z const>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Z const>, int&&>);


        STATIC_CHECK(flux::size(zipped) == 0);
        STATIC_CHECK(flux::is_last(zipped, flux::last(zipped)));
        STATIC_CHECK(flux::equal(flux::empty<int>, zipped));
    }


    return true;
}
static_assert(test_zip_map());

}

TEST_CASE("zip_map")
{
    bool result = test_zip_map();
    REQUIRE(result);
}