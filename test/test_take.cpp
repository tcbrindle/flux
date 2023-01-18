
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>

namespace {

struct Tester : flux::simple_sequence_base<Tester> {

    int i = 0;

    constexpr auto maybe_next() -> std::optional<int>
    {
        return {i++};
    }
};

constexpr bool test_take()
{
    {
        int arr[] = {0, 1, 2, 3, 4};
        auto taken = flux::take(std::ref(arr), 3);

        using T = decltype(taken);
        static_assert(flux::contiguous_sequence<T>);
        static_assert(flux::bounded_sequence<T>);
        static_assert(flux::sized_sequence<T>);

        static_assert(flux::contiguous_sequence<T const>);
        static_assert(flux::bounded_sequence<T const>);
        static_assert(flux::sized_sequence<T const>);

        STATIC_CHECK(taken.size() == 3);
        STATIC_CHECK(check_equal(taken, {0, 1, 2}));
    }

    {
        auto taken = flux::take(std::array{0, 1, 2, 3, 4}, 3);

        using T = decltype(taken);
        static_assert(flux::contiguous_sequence<T>);
        static_assert(flux::bounded_sequence<T>);
        static_assert(flux::sized_sequence<T>);

        static_assert(flux::contiguous_sequence<T const>);
        static_assert(flux::bounded_sequence<T const>);
        static_assert(flux::sized_sequence<T const>);

        STATIC_CHECK(taken.size() == 3);
        STATIC_CHECK(check_equal(taken, {0, 1, 2}));
    }

    {
        auto taken = Tester{}.take(3);

        using T = decltype(taken);
        static_assert(flux::sequence<T>);
        static_assert(not flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        static_assert(not flux::sequence<T const>);

        STATIC_CHECK(check_equal(taken, {0, 1, 2}));
    }

    return true;
}
static_assert(test_take());

}

TEST_CASE("take")
{
    bool result = test_take();
    REQUIRE(result);
}