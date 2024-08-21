
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <tuple>

#include "test_utils.hpp"

namespace {

constexpr bool test_cartesian_power()
{
    // cartesian_power<0> should be empty ( same as cartesian_product<>() )
    {
        auto cart = flux::cartesian_power<0>(std::array{100, 200, 300});
        using C = decltype(cart);
        static_assert(std::is_same_v<flux::value_t<C>, std::tuple<>>);

        STATIC_CHECK(cart.is_empty());
    }
    // cartesian_power<1> should be the same as cartesian_product<T>()
    {
        auto cart = flux::cartesian_power<1>(std::array{100, 200, 300});

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

        STATIC_CHECK(check_equal(cart, {
            std::tuple{100},
            std::tuple{200},
            std::tuple{300},
        }));

        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 3);

        {
            auto cur = flux::next(cart, cart.first());
            flux::inc(cart, cur);
            STATIC_CHECK(cart[cur] == std::tuple{300});
            flux::inc(cart, cur, 0);
            STATIC_CHECK(cart[cur] == std::tuple{300});
            flux::inc(cart, cur, -2);
            STATIC_CHECK(cart[cur] == std::tuple{100});
        }

        int sum = 0;
        cart.for_each(flux::unpack([&] (int x) {
            sum += x;
        }));
        STATIC_CHECK(sum == (100 + 200 + 300));

    }
    {
        auto cart = flux::cartesian_power<2>(std::array{100, 200, 300});

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

        static_assert(std::same_as<flux::element_t<C>, std::tuple<int&, int&>>);
        static_assert(std::same_as<flux::value_t<C>, std::tuple<int, int>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::tuple<int&&, int&&>>);

        static_assert(std::same_as<flux::element_t<C const>, std::tuple<int const&, int const&>>);
        static_assert(std::same_as<flux::value_t<C const>, std::tuple<int, int>>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, std::tuple<int const&&, int const&&>>);

        STATIC_CHECK(flux::size(cart) == 9);

        STATIC_CHECK(check_equal(cart, {
            std::tuple{100, 100},
            std::tuple{100, 200},
            std::tuple{100, 300},
            std::tuple{200, 100},
            std::tuple{200, 200},
            std::tuple{200, 300},
            std::tuple{300, 100},
            std::tuple{300, 200},
            std::tuple{300, 300},
        }));

        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 9);

        {
            auto cur = flux::next(cart, cart.first());
            flux::inc(cart, cur);
            STATIC_CHECK(cart[cur] == std::tuple{100, 300});
            flux::inc(cart, cur, 0);
            STATIC_CHECK(cart[cur] == std::tuple{100, 300});
            flux::inc(cart, cur, -2);
            STATIC_CHECK(cart[cur] == std::tuple{100, 100});
        }

        int sum_i = 0;
        int sum_j = 0;
        cart.for_each(flux::unpack([&] (int i, int j) {
            sum_i += i;
            sum_j += j;
        }));
        STATIC_CHECK(sum_i == 3 * (100 + 200 + 300));
        STATIC_CHECK(sum_j == 3 * (100 + 200 + 300));

    }

    {
        auto prod = flux::cartesian_power<3>(std::array{'a', 'b', 'c', 'd'});
        static_assert(prod.size() == 64);

        using C = decltype(prod);

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

        static_assert(std::same_as<flux::element_t<C>, std::tuple<char&, char&, char&>>);
        static_assert(std::same_as<flux::value_t<C>, std::tuple<char, char, char>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::tuple<char&&, char&&, char&&>>);

        static_assert(std::same_as<flux::element_t<C const>, std::tuple<char const&, char const&, char const&>>);
        static_assert(std::same_as<flux::value_t<C const>, std::tuple<char, char, char>>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, std::tuple<char const&&, char const&&, char const&&>>);

        STATIC_CHECK(flux::size(prod) == 64);

        STATIC_CHECK(check_equal(prod, {
            std::tuple{'a', 'a', 'a'},
            std::tuple{'a', 'a', 'b'},
            std::tuple{'a', 'a', 'c'},
            std::tuple{'a', 'a', 'd'},
            std::tuple{'a', 'b', 'a'},
            std::tuple{'a', 'b', 'b'},
            std::tuple{'a', 'b', 'c'},
            std::tuple{'a', 'b', 'd'},
            std::tuple{'a', 'c', 'a'},
            std::tuple{'a', 'c', 'b'},
            std::tuple{'a', 'c', 'c'},
            std::tuple{'a', 'c', 'd'},
            std::tuple{'a', 'd', 'a'},
            std::tuple{'a', 'd', 'b'},
            std::tuple{'a', 'd', 'c'},
            std::tuple{'a', 'd', 'd'},
            std::tuple{'b', 'a', 'a'},
            std::tuple{'b', 'a', 'b'},
            std::tuple{'b', 'a', 'c'},
            std::tuple{'b', 'a', 'd'},
            std::tuple{'b', 'b', 'a'},
            std::tuple{'b', 'b', 'b'},
            std::tuple{'b', 'b', 'c'},
            std::tuple{'b', 'b', 'd'},
            std::tuple{'b', 'c', 'a'},
            std::tuple{'b', 'c', 'b'},
            std::tuple{'b', 'c', 'c'},
            std::tuple{'b', 'c', 'd'},
            std::tuple{'b', 'd', 'a'},
            std::tuple{'b', 'd', 'b'},
            std::tuple{'b', 'd', 'c'},
            std::tuple{'b', 'd', 'd'},
            std::tuple{'c', 'a', 'a'},
            std::tuple{'c', 'a', 'b'},
            std::tuple{'c', 'a', 'c'},
            std::tuple{'c', 'a', 'd'},
            std::tuple{'c', 'b', 'a'},
            std::tuple{'c', 'b', 'b'},
            std::tuple{'c', 'b', 'c'},
            std::tuple{'c', 'b', 'd'},
            std::tuple{'c', 'c', 'a'},
            std::tuple{'c', 'c', 'b'},
            std::tuple{'c', 'c', 'c'},
            std::tuple{'c', 'c', 'd'},
            std::tuple{'c', 'd', 'a'},
            std::tuple{'c', 'd', 'b'},
            std::tuple{'c', 'd', 'c'},
            std::tuple{'c', 'd', 'd'},
            std::tuple{'d', 'a', 'a'},
            std::tuple{'d', 'a', 'b'},
            std::tuple{'d', 'a', 'c'},
            std::tuple{'d', 'a', 'd'},
            std::tuple{'d', 'b', 'a'},
            std::tuple{'d', 'b', 'b'},
            std::tuple{'d', 'b', 'c'},
            std::tuple{'d', 'b', 'd'},
            std::tuple{'d', 'c', 'a'},
            std::tuple{'d', 'c', 'b'},
            std::tuple{'d', 'c', 'c'},
            std::tuple{'d', 'c', 'd'},
            std::tuple{'d', 'd', 'a'},
            std::tuple{'d', 'd', 'b'},
            std::tuple{'d', 'd', 'c'},
            std::tuple{'d', 'd', 'd'}
        }));

        STATIC_CHECK(flux::distance(prod, prod.first(), prod.last()) == 64);

        {
            auto cur = flux::next(prod, prod.first(), 2);
            STATIC_CHECK(prod[cur] == std::tuple{'a', 'a', 'c'});
            flux::inc(prod, cur, -2);
            STATIC_CHECK(prod[cur] == std::tuple{'a', 'a', 'a'});
        }
    }

    return true;
}

static_assert(test_cartesian_power());

// https://github.com/tcbrindle/flux/issues/177
constexpr bool issue_177()
{
    auto seq = flux::cartesian_power<3>(flux::empty<int>);

    STATIC_CHECK(seq.is_empty());
    STATIC_CHECK(seq.size() == 0);
    STATIC_CHECK(seq.is_last(seq.first()));
    STATIC_CHECK(seq.first() == seq.last());

    return true;
}
static_assert(issue_177());

}

TEST_CASE("cartesian power")
{
    REQUIRE(test_cartesian_power());
    REQUIRE(issue_177());
}
