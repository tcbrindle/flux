
#include "catch.hpp"

#include <flux/op/cartesian_product.hpp>
#include <flux/op/reverse.hpp>
#include <flux/source/iota.hpp>
#include <flux/source/empty.hpp>
#include <flux/source/iota.hpp>
#include <flux/op/for_each.hpp>

#include <array>
#include <iostream>
#include <string_view>

#include "test_utils.hpp"

namespace {

constexpr bool test_cartesian_product()
{
    {
        std::array arr1{100, 200};
        std::array arr2{1.0f, 2.0f};

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

        static_assert(std::same_as<flux::element_t<C>, std::tuple<int&, float&>>);
        static_assert(std::same_as<flux::value_t<C>, std::tuple<int, float>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::tuple<int&&, float&&>>);

        static_assert(std::same_as<flux::element_t<C const>, std::tuple<int&, float&>>);
        static_assert(std::same_as<flux::value_t<C const>, std::tuple<int, float>>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, std::tuple<int&&, float&&>>);

        STATIC_CHECK(flux::size(cart) == 2 * 2);

        STATIC_CHECK(check_equal(cart, {
            std::tuple{100, 1.0f}, std::tuple{100, 2.0f},
            std::tuple{200, 1.0f}, std::tuple{200, 2.0f} }));

        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 2 * 2);

        {
            auto cur = flux::next(cart, cart.first(), 2);
            STATIC_CHECK(cart[cur] == std::tuple{200, 1.0f});
            flux::inc(cart, cur, -2);
            STATIC_CHECK(cart[cur] == std::tuple{100, 1.0f});
        }
    }

    {
        auto cart = flux::cartesian_product(std::array{100, 200}, std::array{1.0f, 2.0f});

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

        static_assert(std::same_as<flux::element_t<C>, std::tuple<int&, float&>>);
        static_assert(std::same_as<flux::value_t<C>, std::tuple<int, float>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::tuple<int&&, float&&>>);

        static_assert(std::same_as<flux::element_t<C const>, std::tuple<int const&, float const&>>);
        static_assert(std::same_as<flux::value_t<C const>, std::tuple<int, float>>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, std::tuple<int const&&, float const&&>>);

        STATIC_CHECK(flux::size(cart) == 2 * 2);

        STATIC_CHECK(check_equal(cart, {
            std::tuple{100, 1.0f}, std::tuple{100, 2.0f},
            std::tuple{200, 1.0f}, std::tuple{200, 2.0f} }));

        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 2 * 2);

        {
            auto cur = flux::next(cart, cart.first(), 2);
            STATIC_CHECK(cart[cur] == std::tuple{200, 1.0f});
            flux::inc(cart, cur, -2);
            STATIC_CHECK(cart[cur] == std::tuple{100, 1.0f});
        }
    }

    {
        std::array arr1{100, 200};
        std::array arr2{1.0f, 2.0f, 3.0f, 4.0f};
        std::array arr3{0ULL, 2ULL, 4ULL};

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

        static_assert(std::same_as<flux::element_t<C>, std::tuple<int&, float&, unsigned long long&>>);
        static_assert(std::same_as<flux::value_t<C>, std::tuple<int, float, unsigned long long>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::tuple<int&&, float&&, unsigned long long&&>>);

        static_assert(std::same_as<flux::element_t<C const>, std::tuple<int&, float&, unsigned long long&>>);
        static_assert(std::same_as<flux::value_t<C const>, std::tuple<int, float, unsigned long long>>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, std::tuple<int&&, float&&, unsigned long long&&>>);

        STATIC_CHECK(flux::size(cart) == 2 * 4 * 3);

        STATIC_CHECK(check_equal(cart, {
            std::tuple{100, 1.0f, 0ULL},
            std::tuple{100, 1.0f, 2ULL},
            std::tuple{100, 1.0f, 4ULL},
            std::tuple{100, 2.0f, 0ULL},
            std::tuple{100, 2.0f, 2ULL},
            std::tuple{100, 2.0f, 4ULL},
            std::tuple{100, 3.0f, 0ULL},
            std::tuple{100, 3.0f, 2ULL},
            std::tuple{100, 3.0f, 4ULL},
            std::tuple{100, 4.0f, 0ULL},
            std::tuple{100, 4.0f, 2ULL},
            std::tuple{100, 4.0f, 4ULL},
            std::tuple{200, 1.0f, 0ULL},
            std::tuple{200, 1.0f, 2ULL},
            std::tuple{200, 1.0f, 4ULL},
            std::tuple{200, 2.0f, 0ULL},
            std::tuple{200, 2.0f, 2ULL},
            std::tuple{200, 2.0f, 4ULL},
            std::tuple{200, 3.0f, 0ULL},
            std::tuple{200, 3.0f, 2ULL},
            std::tuple{200, 3.0f, 4ULL},
            std::tuple{200, 4.0f, 0ULL},
            std::tuple{200, 4.0f, 2ULL},
            std::tuple{200, 4.0f, 4ULL}
        }));

        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 2 * 4 * 3);

        {
            auto cur = flux::next(cart, cart.first(), 3);
            STATIC_CHECK(cart[cur] == std::tuple{100, 2.0f, 0ULL});
            flux::inc(cart, cur, -3);
            STATIC_CHECK(cart[cur] == std::tuple{100, 1.0f, 0ULL});
        }
    }

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

        static_assert(std::same_as<flux::element_t<C>, std::tuple<long, long, long>>);
        static_assert(std::same_as<flux::value_t<C>, std::tuple<long, long, long>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::tuple<long, long, long>>);

        static_assert(std::same_as<flux::element_t<C const>, std::tuple<long, long, long>>);
        static_assert(std::same_as<flux::value_t<C const>, std::tuple<long, long, long>>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, std::tuple<long, long, long>>);

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
    }

    // TODO: Product with a zero-sized sequence works and produces an empty sequence

    // Test unpack()
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

}

TEST_CASE("cartesian_product")
{
    REQUIRE(test_cartesian_product());
}
