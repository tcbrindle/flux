
#include "catch.hpp"

#include <flux/op/cartesian_product.hpp>
#include <flux/op/reverse.hpp>
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