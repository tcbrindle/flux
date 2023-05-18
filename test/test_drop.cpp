
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>
#include <list>

namespace {

constexpr bool test_drop() {

    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        auto dropped = flux::drop(flux::ref(arr), 5);

        using D = decltype(dropped);

        static_assert(flux::contiguous_sequence<D>);
        static_assert(flux::sized_sequence<D>);
        static_assert(flux::bounded_sequence<D>);

        STATIC_CHECK(flux::size(dropped) == 5);
        STATIC_CHECK(flux::data(dropped) == arr + 5);
        STATIC_CHECK(check_equal(dropped, {5, 6, 7, 8, 9}));
    }

    {
        auto dropped = flux::from(std::array{0, 1, 2, 3, 4, 5, 6, 7, 8, 9})
                           .drop(5);

        using D = decltype(dropped);

        static_assert(flux::contiguous_sequence<D>);
        static_assert(flux::sized_sequence<D>);
        static_assert(flux::bounded_sequence<D>);

        STATIC_CHECK(flux::size(dropped) == 5);
        STATIC_CHECK(check_equal(dropped, {5, 6, 7, 8, 9}));
    }

    {
        auto dropped = single_pass_only(flux::from(std::array{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}))
                           .drop(5);

        using D = decltype(dropped);

        static_assert(flux::sequence<D>);
        static_assert(not flux::multipass_sequence<D>);
        static_assert(flux::sized_sequence<D>);
        static_assert(flux::bounded_sequence<D>);

        STATIC_CHECK(flux::size(dropped) == 5);
        STATIC_CHECK(check_equal(dropped, {5, 6, 7, 8, 9}));
    }

    // test dropping zero items
    {
        auto dropped = flux::drop(std::array{1, 2, 3, 4, 5}, 0);

        STATIC_CHECK(dropped.size() == 5);
        STATIC_CHECK(check_equal(dropped, {1, 2, 3, 4, 5}));
    }

    // test dropping all items
    {
        auto const arr = std::array{1, 2, 3, 4, 5};

        auto dropped = flux::ref(arr).drop(5);

        STATIC_CHECK(dropped.is_empty());
        STATIC_CHECK(dropped.size() == 0);
        STATIC_CHECK(dropped.distance(dropped.first(), dropped.last()) == 0);
        STATIC_CHECK(flux::equal(dropped, flux::empty<int>));
        STATIC_CHECK(dropped.data() == arr.data() + 5);
    }

    // test dropping too many items
    {
        auto const arr = std::array{1, 2, 3, 4, 5};

        auto dropped = flux::ref(arr).drop(1000UL);

        STATIC_CHECK(dropped.is_empty());
        STATIC_CHECK(dropped.size() == 0);
        STATIC_CHECK(dropped.distance(dropped.first(), dropped.last()) == 0);
        STATIC_CHECK(flux::equal(dropped, flux::empty<int>));
        STATIC_CHECK(dropped.data() == arr.data() + 5);
    }

    return true;
}
static_assert(test_drop());

}

TEST_CASE("drop")
{
    bool result = test_drop();
    REQUIRE(result);

    // Test dropping a negative number of elements
    {
        std::list list{1, 2, 3, 4, 5};

        REQUIRE_THROWS_AS(flux::drop(flux::from_range(list), -1), flux::unrecoverable_error);

        REQUIRE_THROWS_AS(flux::from_range(list).drop(-1000), flux::unrecoverable_error);
    }
}