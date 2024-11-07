// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <chrono>

#include "test_utils.hpp"

namespace {

constexpr bool test_iota_basic()
{
    auto f = flux::ints();

    using F = decltype(f);

    static_assert(sizeof(f) == 1);
    static_assert(flux::sequence<F>);
    static_assert(flux::bidirectional_sequence<F>);
    static_assert(flux::random_access_sequence<F>);
    static_assert(flux::infinite_sequence<F>);
    static_assert(not flux::bounded_sequence<F>);
    static_assert(not flux::sized_iterable<F>);

    STATIC_CHECK(check_equal(flux::take(f, 5), {0, 1, 2, 3, 4}));

    return true;
}
static_assert(test_iota_basic());

constexpr bool test_iota_from()
{
    auto f = flux::iota(1u);

    using F = decltype(f);

    static_assert(sizeof(f) == sizeof(unsigned));
    static_assert(flux::sequence<F>);
    static_assert(flux::bidirectional_sequence<F>);
    static_assert(flux::random_access_sequence<F>);
    static_assert(flux::infinite_sequence<F>);
    static_assert(not flux::bounded_sequence<F>);
    static_assert(not flux::sized_iterable<F>);

    STATIC_CHECK(check_equal(flux::take(f, 5), {1u, 2u, 3u, 4u, 5u}));

    return true;
}
static_assert(test_iota_from());

constexpr bool test_iota_bounded()
{
    auto f = flux::iota(1u, 6u);

    using F = decltype(f);

    static_assert(flux::sequence<F>);
    static_assert(flux::bidirectional_sequence<F>);
    static_assert(flux::random_access_sequence<F>);
    static_assert(not flux::infinite_sequence<F>);
    static_assert(flux::bounded_sequence<F>);
    static_assert(flux::sized_iterable<F>);

    STATIC_CHECK(f.size() == 5);
    STATIC_CHECK(check_equal(f, {1u, 2u, 3u, 4u, 5u}));

    return true;
}
static_assert(test_iota_bounded());

constexpr bool test_iota_custom_type()
{
    using namespace std::chrono_literals;

    auto f = flux::iota(1s, 6s);

    using F = decltype(f);

    static_assert(flux::sequence<F>);
    static_assert(flux::bidirectional_sequence<F>);
    static_assert(not flux::random_access_sequence<F>); // no iter_difference_t
    static_assert(not flux::infinite_sequence<F>);
    static_assert(flux::bounded_sequence<F>);
    static_assert(not flux::sized_iterable<F>); // !

    STATIC_CHECK(f.count() == 5);
    STATIC_CHECK(check_equal(f, {1s, 2s, 3s, 4s, 5s}));

    return true;
}
static_assert(test_iota_custom_type());

}

TEST_CASE("iota")
{
    REQUIRE(test_iota_basic());
    REQUIRE(test_iota_from());
    REQUIRE(test_iota_bounded());
    REQUIRE(test_iota_custom_type());
}
