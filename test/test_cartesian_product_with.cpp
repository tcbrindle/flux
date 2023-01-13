
#include "catch.hpp"

#include <flux/op/cartesian_product_with.hpp>
#include <flux/op/reverse.hpp>
#include <flux/source/empty.hpp>
#include <flux/op/for_each.hpp>

#include <array>
#include <iostream>

#include "test_utils.hpp"

namespace {

constexpr auto sum = [](auto... args) { return (args + ...); };

constexpr bool test_cartesian_product_with()
{
    {
        std::array arr1{100, 200};
        std::array arr2{1, 2, 3, 4, 5};

        auto cart = flux::cartesian_product_with(sum, flux::ref(arr1), flux::ref(arr2));
        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        STATIC_CHECK(flux::size(cart) == 2 * 5);

        STATIC_CHECK(check_equal(cart, { 101, 102, 103, 104, 105,
                                         201, 202, 203, 204, 205
                                       }));

        STATIC_CHECK(check_equal(flux::reverse(flux::ref(cart)),
                                 { 205, 204, 203, 202, 201,
                                   105, 104, 103, 102, 101
                                 }));

        // Random access checks
        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 2 * 5);

        {
            auto cur = flux::next(cart, cart.first(), 7);
            STATIC_CHECK(cart[cur] == 203);
            flux::inc(cart, cur, -7);
            STATIC_CHECK(cart[cur] == 101);
        }
    }

    {
        std::array arr1{100, 200};
        std::array arr2{10, 20, 30};
        std::array arr3{1, 2, 3, 4};

        auto cart = flux::cartesian_product_with(sum, flux::ref(arr1), flux::ref(arr2), flux::ref(arr3));
        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        STATIC_CHECK(flux::size(cart) == 2 * 3 * 4);

        STATIC_CHECK(check_equal(cart, { 111, 112, 113, 114,
                                         121, 122, 123, 124,
                                         131, 132, 133, 134,
                                         211, 212, 213, 214,
                                         221, 222, 223, 224,
                                         231, 232, 233, 234 }
        ));

        {
            auto cur = flux::next(cart, cart.first(), 7);
            STATIC_CHECK(cart[cur] == 124);
            cur = flux::next(cart, cart.first(), 19);
            STATIC_CHECK(cart[cur] == 224);
            cur = flux::next(cart, cur, -19);
            STATIC_CHECK(cart[cur] == 111);
        }
    }

    {
        auto seq0 = single_pass_only(flux::from(std::array{100, 200}));
        auto cart = flux::cartesian_product_with(sum, std::move(seq0), std::array{1, 2, 3});
        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(not flux::multipass_sequence<C>);

        STATIC_CHECK(flux::size(cart) == 2 * 3);

        STATIC_CHECK(check_equal(cart, {101, 102, 103, 201, 202, 203}));
    }

    // Product with a zero-sized sequence works and produces an empty sequence
    {
        auto arr = std::array{1, 2, 3, 4, 5};
        auto emp = flux::empty<int>;

        auto cart = flux::cartesian_product_with(sum, flux::ref(arr), std::move(emp));

        static_assert(flux::bidirectional_sequence<decltype(cart)>);

        STATIC_CHECK(cart.is_empty());

        int s = 0;
        cart.for_each([&s](int i) { s += i; });
        STATIC_CHECK(s == 0);
    }

    return true;
}
static_assert(test_cartesian_product_with());

TEST_CASE("cartesian_product_with")
{
    REQUIRE(test_cartesian_product_with());
}

}