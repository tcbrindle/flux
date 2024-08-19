
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <iostream>
#include <list>

#include "test_utils.hpp"

namespace {

constexpr bool test_zip()
{
    {
        int arr1[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        double arr2[] = {0, 100, 200, 300, 400};

        auto zipped = flux::zip(flux::mut_ref(arr1), flux::mut_ref(arr2));

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

        static_assert(std::same_as<flux::element_t<Z>, std::pair<int&, double&>>);
        static_assert(std::same_as<flux::value_t<Z>, std::pair<int, double>>);
        static_assert(std::same_as<flux::rvalue_element_t<Z>, std::pair<int&&, double&&>>);

        // Believe it or not these are correct -- we've zipped two ref adaptors
        static_assert(std::same_as<flux::element_t<Z const>, std::pair<int&, double&>>);
        static_assert(std::same_as<flux::value_t<Z const>, std::pair<int, double>>);
        static_assert(std::same_as<flux::rvalue_element_t<Z const>, std::pair<int&&, double&&>>);


        STATIC_CHECK(flux::size(zipped) == 5);
        STATIC_CHECK(flux::is_last(zipped, flux::last(zipped)));

        {
            int n = 0;
            FLUX_FOR(auto p, zipped) {
                auto [a, b] = p;
                STATIC_CHECK(a == n);
                STATIC_CHECK(b == 100.0 * n);
                ++n;
            }
        }
    }

    {
        auto zipped = flux::zip(std::array{0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                                std::array{0.0, 100.0, 200.0, 300.0, 400.0});

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

        static_assert(std::same_as<flux::element_t<Z>, std::pair<int&, double&>>);
        static_assert(std::same_as<flux::value_t<Z>, std::pair<int, double>>);
        static_assert(std::same_as<flux::rvalue_element_t<Z>, std::pair<int&&, double&&>>);

        static_assert(std::same_as<flux::element_t<Z const>, std::pair<int const&, double const&>>);
        static_assert(std::same_as<flux::value_t<Z const>, std::pair<int, double>>);
        static_assert(std::same_as<flux::rvalue_element_t<Z const>, std::pair<int const&&, double const&&>>);

        STATIC_CHECK(flux::size(zipped) == 5);

        {
            int n = 0;
            FLUX_FOR(auto p, zipped) {
                auto [a, b] = p;
                STATIC_CHECK(a == n);
                STATIC_CHECK(b == 100.0 * n);
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

        auto zipped = flux::zip(flux::mut_ref(arr1), flux::mut_ref(arr2));

        using Z = decltype(zipped);
        static_assert(flux::random_access_sequence<Z>);
        static_assert(flux::sized_sequence<Z>);

        auto cur1 = flux::next(zipped, flux::first(zipped), 2);
        auto cur2 = flux::next(zipped, flux::first(zipped), 3);

        flux::swap_at(zipped, cur1, cur2);

        STATIC_CHECK(check_equal(arr1, {1, 2, 4, 3, 5}));
        STATIC_CHECK(check_equal(arr2, {100, 200, 400, 300, 500}));
    }

    {
        std::array arr1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::array arr2 = {0, 100, 200, 300, 400};
        std::array arr3 = {'o', 'l', 'l', 'e', 'h', '\0'};

        flux::inplace_reverse(flux::zip(flux::mut_ref(arr1), flux::mut_ref(arr2), flux::mut_ref(arr3)));

        STATIC_CHECK(arr1 == std::array{4, 3, 2, 1, 0, 5, 6, 7, 8, 9});
        STATIC_CHECK(arr2 == std::array{400, 300, 200, 100, 0});
        STATIC_CHECK(std::string_view(arr3.data()) == "hello");
    }

    {
        std::array arr1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::array arr2 = {0.0, 100.0, 200.0, 300.0, 400.0};
        std::array arr3 = {'o', 'l', 'l', 'e', 'h', '\0'};

        auto view = flux::zip(flux::mut_ref(arr1), flux::mut_ref(arr2), flux::mut_ref(arr3));

        using V = decltype(view);

        namespace rng = std::ranges;

        static_assert(rng::random_access_range<V>);
        static_assert(not rng::contiguous_range<V>);
        static_assert(rng::sized_range<V>);
        static_assert(rng::common_range<V>);

        static_assert(std::same_as<rng::range_reference_t<V>, std::tuple<int&, double&, char&>>);
        static_assert(std::same_as<rng::range_value_t<V>, std::tuple<int, double, char>>);
        static_assert(std::same_as<rng::range_rvalue_reference_t<V>, std::tuple<int&&, double&&, char&&>>);

        STATIC_CHECK(view.size() == 5);
    }

    // test unpack()
    {
        auto vals = std::array{0, 1, 2, 3, 4};
        auto words = std::array<std::string_view, 3>{"0", "1", "2"};

        flux::zip(flux::mut_ref(vals), words)
            .map(flux::unpack([](int& val, std::string_view str) -> int& {
                if (str[0] - '0' != val) {
                    throw std::runtime_error("Something has gone wrong");
                }
                return val;
            }))
            .fill(100);

        STATIC_CHECK(check_equal(vals, {100, 100, 100, 3, 4}));
    }

    return true;
}
static_assert(test_zip());

// https://github.com/tcbrindle/flux/issues/47
constexpr bool issue_47()
{
    std::array v = {1, 2, 3, 4, 5};
    auto res = flux::zip(flux::ints(), flux::from(v).filter(flux::pred::gt(3)));

    for ([[maybe_unused]] auto [a, b] : res) {}

    return true;
}
static_assert(issue_47());

}

TEST_CASE("zip")
{
    bool result = test_zip();
    REQUIRE(result);

    result = issue_47();
    REQUIRE(result);
}