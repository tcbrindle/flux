
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>
#include <iostream>
#include <string_view>

#include "test_utils.hpp"

namespace {

constexpr bool test_cartesian_product()
{
    // 1D `cartesian_product`.
    {
        auto cart = flux::cartesian_product(std::array{100, 200, 300});

        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(not flux::contiguous_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        static_assert(flux::sequence<C const>);
        static_assert(flux::multipass_sequence<C const>);
        static_assert(flux::bidirectional_sequence<C const>);
        static_assert(flux::random_access_sequence<C const>);
        static_assert(not flux::contiguous_sequence<C const>);
        static_assert(flux::bounded_sequence<C const>);
        static_assert(flux::sized_sequence<C const>);

        static_assert(std::same_as<flux::element_t<C>, std::tuple<int&>>);
        static_assert(std::same_as<flux::value_t<C>, std::tuple<int>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::tuple<int&&>>);

        static_assert(std::same_as<flux::element_t<C const>, std::tuple<int const&>>);
        static_assert(std::same_as<flux::value_t<C const>, std::tuple<int>>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, std::tuple<int const&&>>);

        STATIC_CHECK(flux::size(cart) == 3);

        STATIC_CHECK(check_equal(cart, {std::tuple{100}, std::tuple{200}, std::tuple{300}}));

        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 3);

        {
            auto cur = flux::next(cart, cart.first(), 2);
            STATIC_CHECK(cart[cur] == std::tuple{300});
            flux::inc(cart, cur, -2);
            STATIC_CHECK(cart[cur] == std::tuple{100});
        }

        int sum = 0;
        cart.for_each(flux::unpack([&] (int i) { sum += i; }));
        STATIC_CHECK(sum == 100 + 200 + 300);
    }

    // 2D `cartesian_product` with lvalue references.
    {
        std::array arr1{100, 200};
        std::array arr2{true, false};

        auto cart = flux::cartesian_product(flux::mut_ref(arr1), flux::mut_ref(arr2));

        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(not flux::contiguous_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        static_assert(flux::sequence<C const>);
        static_assert(flux::multipass_sequence<C const>);
        static_assert(flux::bidirectional_sequence<C const>);
        static_assert(flux::random_access_sequence<C const>);
        static_assert(not flux::contiguous_sequence<C const>);
        static_assert(flux::bounded_sequence<C const>);
        static_assert(flux::sized_sequence<C const>);

        static_assert(std::same_as<flux::element_t<C>, std::tuple<int&, bool&>>);
        static_assert(std::same_as<flux::value_t<C>, std::tuple<int, bool>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::tuple<int&&, bool&&>>);

        static_assert(std::same_as<flux::element_t<C const>, std::tuple<int&, bool&>>);
        static_assert(std::same_as<flux::value_t<C const>, std::tuple<int, bool>>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, std::tuple<int&&, bool&&>>);

        STATIC_CHECK(flux::size(cart) == 2 * 2);

        STATIC_CHECK(check_equal(cart, {
            std::tuple{100, true}, std::tuple{100, false},
            std::tuple{200, true}, std::tuple{200, false} }));

        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 2 * 2);

        {
            auto cur = flux::next(cart, cart.first(), 2);
            STATIC_CHECK(cart[cur] == std::tuple{200, true});
            flux::inc(cart, cur, -2);
            STATIC_CHECK(cart[cur] == std::tuple{100, true});
        }

        int sum_i = 0;
        int sum_j = 0;
        cart.for_each(flux::unpack([&] (int i, bool j) {
                sum_i += i;
                sum_j += j;
            }));
        STATIC_CHECK(sum_i == 2 * (100 + 200));
        STATIC_CHECK(sum_j == 2);
    }

    // 2D `cartesian_product` with rvalue references and temporaries.
    {
        auto cart = flux::cartesian_product(std::array{100, 200}, std::array{true, false});

        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(not flux::contiguous_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        static_assert(flux::sequence<C const>);
        static_assert(flux::multipass_sequence<C const>);
        static_assert(flux::bidirectional_sequence<C const>);
        static_assert(flux::random_access_sequence<C const>);
        static_assert(not flux::contiguous_sequence<C const>);
        static_assert(flux::bounded_sequence<C const>);
        static_assert(flux::sized_sequence<C const>);

        static_assert(std::same_as<flux::element_t<C>, std::tuple<int&, bool&>>);
        static_assert(std::same_as<flux::value_t<C>, std::tuple<int, bool>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::tuple<int&&, bool&&>>);

        static_assert(std::same_as<flux::element_t<C const>, std::tuple<int const&, bool const&>>);
        static_assert(std::same_as<flux::value_t<C const>, std::tuple<int, bool>>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, std::tuple<int const&&, bool const&&>>);

        STATIC_CHECK(flux::size(cart) == 2 * 2);

        STATIC_CHECK(check_equal(cart, {
            std::tuple{100, true}, std::tuple{100, false},
            std::tuple{200, true}, std::tuple{200, false} }));

        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 2 * 2);

        {
            auto cur = flux::next(cart, cart.first(), 2);
            STATIC_CHECK(cart[cur] == std::tuple{200, true});
            flux::inc(cart, cur, -2);
            STATIC_CHECK(cart[cur] == std::tuple{100, true});
        }

        int sum_i = 0;
        int sum_j = 0;
        cart.for_each(flux::unpack([&] (int i, bool j) {
                sum_i += i;
                sum_j += j;
            }));
        STATIC_CHECK(sum_i == 2 * (100 + 200));
        STATIC_CHECK(sum_j == 2);
    }

    // 3D `cartesian_product`.
    {
        std::array arr1{100, 200};
        std::array arr2{true, false, true, false};
        std::array arr3{1ULL, 2ULL, 4ULL};

        auto cart = flux::cartesian_product(flux::mut_ref(arr1), flux::mut_ref(arr2), flux::mut_ref(arr3));

        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(not flux::contiguous_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        static_assert(flux::sequence<C const>);
        static_assert(flux::multipass_sequence<C const>);
        static_assert(flux::bidirectional_sequence<C const>);
        static_assert(flux::random_access_sequence<C const>);
        static_assert(not flux::contiguous_sequence<C const>);
        static_assert(flux::bounded_sequence<C const>);
        static_assert(flux::sized_sequence<C const>);

        static_assert(std::same_as<flux::element_t<C>, std::tuple<int&, bool&, unsigned long long&>>);
        static_assert(std::same_as<flux::value_t<C>, std::tuple<int, bool, unsigned long long>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::tuple<int&&, bool&&, unsigned long long&&>>);

        static_assert(std::same_as<flux::element_t<C const>, std::tuple<int&, bool&, unsigned long long&>>);
        static_assert(std::same_as<flux::value_t<C const>, std::tuple<int, bool, unsigned long long>>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, std::tuple<int&&, bool&&, unsigned long long&&>>);

        STATIC_CHECK(flux::size(cart) == 2 * 4 * 3);

        STATIC_CHECK(check_equal(cart, {
            std::tuple{100, true, 1ULL},
            std::tuple{100, true, 2ULL},
            std::tuple{100, true, 4ULL},
            std::tuple{100, false, 1ULL},
            std::tuple{100, false, 2ULL},
            std::tuple{100, false, 4ULL},
            std::tuple{100, true, 1ULL},
            std::tuple{100, true, 2ULL},
            std::tuple{100, true, 4ULL},
            std::tuple{100, false, 1ULL},
            std::tuple{100, false, 2ULL},
            std::tuple{100, false, 4ULL},
            std::tuple{200, true, 1ULL},
            std::tuple{200, true, 2ULL},
            std::tuple{200, true, 4ULL},
            std::tuple{200, false, 1ULL},
            std::tuple{200, false, 2ULL},
            std::tuple{200, false, 4ULL},
            std::tuple{200, true, 1ULL},
            std::tuple{200, true, 2ULL},
            std::tuple{200, true, 4ULL},
            std::tuple{200, false, 1ULL},
            std::tuple{200, false, 2ULL},
            std::tuple{200, false, 4ULL}
        }));

        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 2 * 4 * 3);

        {
            auto cur = flux::next(cart, cart.first(), 3);
            STATIC_CHECK(cart[cur] == std::tuple{100, false, 1ULL});
            flux::inc(cart, cur, -3);
            STATIC_CHECK(cart[cur] == std::tuple{100, true, 1ULL});
        }

        int sum_i = 0;
        int sum_j = 0;
        unsigned long long sum_k = 0;
        cart.for_each(flux::unpack([&] (int i, bool j, unsigned long long k) {
                sum_i += i;
                sum_j += j;
                sum_k += k;
            }));
        STATIC_CHECK(sum_i == 12 * (100 + 200));
        STATIC_CHECK(sum_j == 12);
        STATIC_CHECK(sum_k == 8ULL * (1ULL + 2ULL + 4ULL));
    }

    // Higher dimension `cartesian_product`.
    {
        std::array arr{100, 200};

        auto cart = flux::cartesian_product(
          flux::mut_ref(arr),
          flux::mut_ref(arr),
          flux::mut_ref(arr),
          flux::mut_ref(arr),
          flux::mut_ref(arr),
          flux::mut_ref(arr)
        );

        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(not flux::contiguous_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        static_assert(flux::sequence<C const>);
        static_assert(flux::multipass_sequence<C const>);
        static_assert(flux::bidirectional_sequence<C const>);
        static_assert(flux::random_access_sequence<C const>);
        static_assert(not flux::contiguous_sequence<C const>);
        static_assert(flux::bounded_sequence<C const>);
        static_assert(flux::sized_sequence<C const>);

        static_assert(std::same_as<flux::element_t<C>, std::tuple<int&, int&, int&, int&, int&, int&>>);
        static_assert(std::same_as<flux::value_t<C>, std::tuple<int, int, int, int, int, int>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::tuple<int&&, int&&, int&&, int&&, int&&, int&&>>);

        static_assert(std::same_as<flux::element_t<C const>, std::tuple<int&, int&, int&, int&, int&, int&>>);
        static_assert(std::same_as<flux::value_t<C const>, std::tuple<int, int, int, int, int, int>>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, std::tuple<int&&, int&&, int&&, int&&, int&&, int&&>>);

        STATIC_CHECK(flux::size(cart) == 2 * 2 * 2 * 2 * 2 * 2);

        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 2 * 2 * 2 * 2 * 2 * 2);

        {
            auto cur = flux::next(cart, cart.first(), 3);
            STATIC_CHECK(cart[cur] == std::tuple{100, 100, 100, 100, 200, 200});
            flux::inc(cart, cur, -3);
            STATIC_CHECK(cart[cur] == std::tuple{100, 100, 100, 100, 100, 100});
        }

        int sum_a = 0;
        int sum_b = 0;
        int sum_c = 0;
        int sum_d = 0;
        int sum_e = 0;
        int sum_f = 0;
        cart.for_each(flux::unpack([&] (int a, int b, int c, int d, int e, int f) {
                sum_a += a;
                sum_b += b;
                sum_c += c;
                sum_d += d;
                sum_e += e;
                sum_f += f;
            }));
        STATIC_CHECK(sum_a == 32 * (100 + 200));
        STATIC_CHECK(sum_b == 32 * (100 + 200));
        STATIC_CHECK(sum_c == 32 * (100 + 200));
        STATIC_CHECK(sum_d == 32 * (100 + 200));
        STATIC_CHECK(sum_e == 32 * (100 + 200));
        STATIC_CHECK(sum_f == 32 * (100 + 200));
    }

    // `cartesian_product` of `iota`/`ints`.
    {
        auto cart = flux::cartesian_product(flux::ints(0, 4), flux::ints(0, 2), flux::ints(0, 3));

        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(not flux::contiguous_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        static_assert(flux::sequence<C const>);
        static_assert(flux::multipass_sequence<C const>);
        static_assert(flux::bidirectional_sequence<C const>);
        static_assert(flux::random_access_sequence<C const>);
        static_assert(not flux::contiguous_sequence<C const>);
        static_assert(flux::bounded_sequence<C const>);
        static_assert(flux::sized_sequence<C const>);

        static_assert(std::same_as<flux::element_t<C>, std::tuple<flux::distance_t, flux::distance_t, flux::distance_t>>);
        static_assert(std::same_as<flux::value_t<C>, std::tuple<flux::distance_t, flux::distance_t, flux::distance_t>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::tuple<flux::distance_t, flux::distance_t, flux::distance_t>>);

        static_assert(std::same_as<flux::element_t<C const>, std::tuple<flux::distance_t, flux::distance_t, flux::distance_t>>);
        static_assert(std::same_as<flux::value_t<C const>, std::tuple<flux::distance_t, flux::distance_t, flux::distance_t>>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, std::tuple<flux::distance_t, flux::distance_t, flux::distance_t>>);

        STATIC_CHECK(flux::size(cart) == 4 * 2 * 3);

        STATIC_CHECK(check_equal(cart, {
            std::tuple{0, 0, 0},
            std::tuple{0, 0, 1},
            std::tuple{0, 0, 2},
            std::tuple{0, 1, 0},
            std::tuple{0, 1, 1},
            std::tuple{0, 1, 2},
            std::tuple{1, 0, 0},
            std::tuple{1, 0, 1},
            std::tuple{1, 0, 2},
            std::tuple{1, 1, 0},
            std::tuple{1, 1, 1},
            std::tuple{1, 1, 2},
            std::tuple{2, 0, 0},
            std::tuple{2, 0, 1},
            std::tuple{2, 0, 2},
            std::tuple{2, 1, 0},
            std::tuple{2, 1, 1},
            std::tuple{2, 1, 2},
            std::tuple{3, 0, 0},
            std::tuple{3, 0, 1},
            std::tuple{3, 0, 2},
            std::tuple{3, 1, 0},
            std::tuple{3, 1, 1},
            std::tuple{3, 1, 2},
        }));

        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 4 * 2 * 3);

        {
            STATIC_CHECK(flux::next(cart, cart.first(), 6) == std::tuple{1, 0, 0});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 6),  1) == std::tuple{1, 0, 1});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 6),  2) == std::tuple{1, 0, 2});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 6),  3) == std::tuple{1, 1, 0});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 6),  4) == std::tuple{1, 1, 1});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 6),  5) == std::tuple{1, 1, 2});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 6), -1) == std::tuple{0, 1, 2});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 6), -2) == std::tuple{0, 1, 1});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 6), -3) == std::tuple{0, 1, 0});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 6), -4) == std::tuple{0, 0, 2});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 6), -5) == std::tuple{0, 0, 1});

            STATIC_CHECK(flux::next(cart, cart.first(), 11) == std::tuple{1, 1, 2});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 11),  1) == std::tuple{2, 0, 0});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 11),  2) == std::tuple{2, 0, 1});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 11),  3) == std::tuple{2, 0, 2});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 11),  4) == std::tuple{2, 1, 0});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 11),  5) == std::tuple{2, 1, 1});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 11), -1) == std::tuple{1, 1, 1});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 11), -2) == std::tuple{1, 1, 0});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 11), -3) == std::tuple{1, 0, 2});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 11), -4) == std::tuple{1, 0, 1});
            STATIC_CHECK(flux::next(cart, flux::next(cart, cart.first(), 11), -5) == std::tuple{1, 0, 0});
        }

        flux::distance_t sum_i = 0;
        flux::distance_t sum_j = 0;
        flux::distance_t sum_k = 0;
        cart.for_each(flux::unpack([&] (flux::distance_t i, flux::distance_t j, flux::distance_t k) {
                sum_i += i;
                sum_j += j;
                sum_k += k;
            }));
        constexpr auto triangular_number = [] (auto n) { return (n * (n + 1)) / 2; };
        STATIC_CHECK(sum_i == triangular_number(4 - 1) * 2 * 3);
        STATIC_CHECK(sum_j == 4 * triangular_number(2 - 1) * 3);
        STATIC_CHECK(sum_k == 4 * 2 * triangular_number(3 - 1));
    }

    // `cartesian_product` `for_each` element type.
    {
        struct T {};

        auto cart = flux::cartesian_product(std::array{100, 200}, std::array{T{}, T{}});

        int sum_i = 0;
        int count_j = 0;
        cart.for_each(flux::unpack([&] (int i, T) {
                sum_i += i;
                count_j += 1;
            }));
        STATIC_CHECK(sum_i == 2 * (100 + 200));
        STATIC_CHECK(count_j == 4);
    }

    // `cartesian_product` `for_each_while` short circuits.
    {
        auto cart = flux::cartesian_product(std::array{100, 200}, std::array{300, 0});

        int count = 0;
        cart.for_each_while(flux::unpack([&] (auto, auto j) {
                ++count;
                return j != 0;
            }));
        STATIC_CHECK(count == 2);
    }

    // `cartesian_product` with a zero-sized sequence produces an empty sequence.
    {
        auto cart = flux::cartesian_product(std::array{1, 2, 3, 4, 5},
                                            flux::empty<int>);

        static_assert(flux::bidirectional_sequence<decltype(cart)>);

        STATIC_CHECK(cart.is_empty());

        int s = 0;
        cart.for_each(flux::unpack([&s](int i, int) { s += i; }));
        STATIC_CHECK(s == 0);
    }

    // `cartesian_product` with `unpack`.
    {
        int vals[3][3] = {};

        flux::cartesian_product(flux::ints(0, 3), flux::ints(0, 3))
            .for_each(flux::unpack([&vals](auto i, auto j) {
                vals[i][j] = 100;
            }));

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                STATIC_CHECK(vals[i][j] == 100);
            }
        }
    }

    return true;
}
static_assert(test_cartesian_product());

// https://github.com/tcbrindle/flux/issues/167
void issue_167()
{
    // Check that overflowing size() is correctly caught
    auto ints = flux::ints(0, std::numeric_limits<flux::distance_t>::max());

    auto prod = flux::cartesian_product(ints, ints, ints);

    REQUIRE_THROWS_AS(flux::size(prod), flux::unrecoverable_error);
}

// https://github.com/tcbrindle/flux/issues/177
constexpr bool issue_177()
{
    auto seq = flux::cartesian_product(std::array{1, 2, 3}, flux::empty<int>);

    STATIC_CHECK(seq.is_empty());
    STATIC_CHECK(seq.size() == 0);
    STATIC_CHECK(seq.is_last(seq.first()));
    STATIC_CHECK(seq.first() == seq.last());

    return true;
}
static_assert(issue_177());

}

TEST_CASE("cartesian_product")
{
    REQUIRE(test_cartesian_product());

    issue_167();
    REQUIRE(issue_177());
}
