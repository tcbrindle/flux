
#include <array>

#include "test_utils.hpp"

namespace {

constexpr auto sum = [](auto... args) { return (args + ...); };

constexpr bool test_cartesian_product_map()
{
    {
        std::array arr1{100, 200};
        std::array arr2{1, 2, 3, 4, 5};

        auto cart =
            flux::cartesian_product_map(sum, flux::ref(arr1), flux::ref(arr2));
        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        STATIC_CHECK(flux::size(cart) == 2 * 5);

        STATIC_CHECK(check_equal(
            cart, {101, 102, 103, 104, 105, 201, 202, 203, 204, 205}));

        STATIC_CHECK(
            check_equal(flux::reverse(flux::ref(cart)),
                        {205, 204, 203, 202, 201, 105, 104, 103, 102, 101}));

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

        auto cart = flux::cartesian_product_map(
            sum, flux::ref(arr1), flux::ref(arr2), flux::ref(arr3));
        using C = decltype(cart);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(flux::sized_sequence<C>);

        STATIC_CHECK(flux::size(cart) == 2 * 3 * 4);

        STATIC_CHECK(
            check_equal(cart, {111, 112, 113, 114, 121, 122, 123, 124,
                               131, 132, 133, 134, 211, 212, 213, 214,
                               221, 222, 223, 224, 231, 232, 233, 234}));

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
        auto cart = flux::cartesian_product_map(sum, std::move(seq0),
                                                std::array{1, 2, 3});
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

        auto cart =
            flux::cartesian_product_map(sum, flux::ref(arr), std::move(emp));

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

        auto seq = flux::cartesian_product_map(get, flux::iota(0, 3),
                                               flux::iota(0, 3));

        static_assert(std::same_as<flux::element_t<decltype(seq)>, double&>);

        seq.fill(100.0);

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) { STATIC_CHECK(vals[i][j] == 100.0); }
        }
    }

    return true;
}
static_assert(test_cartesian_product_map());

// https://github.com/tcbrindle/flux/issues/167
void issue_167()
{
    // Check that overflowing size() is correctly caught
    auto ints = flux::ints(0, std::numeric_limits<flux::int_t>::max());

    auto prod = flux::cartesian_product_map([](auto...) { return 0; }, ints, ints, ints);

    REQUIRE_THROWS_AS(flux::size(prod), flux::unrecoverable_error);
}

}

TEST_CASE("cartesian_product_map")
{
    REQUIRE(test_cartesian_product_map());

    issue_167();
}
