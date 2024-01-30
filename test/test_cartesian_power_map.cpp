
#include "catch.hpp"

#include <array>

#include "test_utils.hpp"

namespace {

constexpr auto sum = [](auto... args) { return (args + ...); };

constexpr bool test_cartesian_power_map()
{
    // cartesian_power_map<0> should be empty ( same as cartesian_product<>() )
    {
        auto make_0_1_pair = [](){ return std::make_pair(0, 1); };
        auto cart = flux::cartesian_power_map<0>(std::array{100, 200, 300}, make_0_1_pair);
        using C = decltype(cart);
        static_assert(std::is_same_v<flux::value_t<C>, std::pair<int, int>>);

        STATIC_CHECK(cart.is_empty());
    }
    // cartesian_power_map<1> should same as map<T, F>()
    {
        std::array arr1{100, 200};

        constexpr auto square_individual = [](auto arg) { return arg * arg; };

        auto cart = flux::cartesian_power_map<1>(flux::ref(arr1), square_individual);
        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        STATIC_CHECK(flux::size(cart) == 2);

        STATIC_CHECK(check_equal(cart, { 10'000, 40'000 }));

        STATIC_CHECK(check_equal(flux::reverse(flux::ref(cart)),
                                 { 40'000, 10'000 }));

        // Random access checks
        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 2);

        {
            auto cur = flux::next(cart, cart.first(), 1);
            STATIC_CHECK(cart[cur] == 40'000);
            flux::inc(cart, cur, 0);
            STATIC_CHECK(cart[cur] == 40'000);
            flux::inc(cart, cur, -1);
            STATIC_CHECK(cart[cur] == 10'000);
        }
    }
    {
        std::array arr1{100, 200};

        auto cart = flux::cartesian_power_map<2>(flux::ref(arr1), sum);
        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        STATIC_CHECK(flux::size(cart) == 2 * 2);

        STATIC_CHECK(check_equal(cart, { 200, 300, 300, 400 }));

        STATIC_CHECK(check_equal(flux::reverse(flux::ref(cart)),
                                 { 400, 300, 300, 200 }));

        // Random access checks
        STATIC_CHECK(flux::distance(cart, cart.first(), cart.last()) == 2 * 2);

        {
            auto cur = flux::next(cart, cart.first(), 3);
            STATIC_CHECK(cart[cur] == 400);
            flux::inc(cart, cur, -3);
            STATIC_CHECK(cart[cur] == 200);
        }
    }

    {
        std::array arr1{1, 3};

        auto cart = flux::cartesian_power_map<3>(flux::ref(arr1), sum);
        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        STATIC_CHECK(flux::size(cart) == 2 * 2 * 2);

        STATIC_CHECK(check_equal(cart, { 3, 5, 5, 7, 5, 7, 7, 9 }));

        {
            auto cur = flux::next(cart, cart.first(), 4);
            STATIC_CHECK(cart[cur] == 5);
            cur = flux::next(cart, cart.first(), 2);
            STATIC_CHECK(cart[cur] == 5);
            cur = flux::next(cart, cur, -2);
            STATIC_CHECK(cart[cur] == 3);
        }
    }


    // Product with a zero-sized sequence works and produces an empty sequence
    {
        auto emp = flux::empty<int>;
        auto cart = flux::cartesian_power_map<5>(emp, sum);

        static_assert(flux::bidirectional_sequence<decltype(cart)>);

        STATIC_CHECK(cart.is_empty());

        int s = 0;
        cart.for_each([&s](int i) { s += i; });
        STATIC_CHECK(s == 0);
    }

    // Product returns a correct reference type
    {
        double vals[3][3] = {};
        auto get = [&vals](auto i, auto j) -> double& { return vals[i][j]; };

        auto seq = flux::cartesian_power_map<2>(flux::iota(0, 3), get);

        static_assert(std::same_as<flux::element_t<decltype(seq)>, double&>);

        seq.fill(100.0);

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                STATIC_CHECK(vals[i][j] == 100.0);
            }
        }
    }

    return true;
}
static_assert(test_cartesian_power_map());

TEST_CASE("cartesian_power_map")
{
    REQUIRE(test_cartesian_power_map());
}

}