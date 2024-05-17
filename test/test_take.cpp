
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>
#include <list>
#include <optional>

#include "test_utils.hpp"

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
        STATIC_CHECK(taken.data() == arr);
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

    // test taking all the elements
    {
        auto arr = std::array{1, 2, 3, 4, 5};

        auto taken = flux::ref(arr).take(5);

        STATIC_CHECK(taken.size() == 5);
        STATIC_CHECK(taken.data() == arr.data());
        STATIC_CHECK(check_equal(taken, arr));
    }

    // test taking "too many" elements
    {
        auto arr = std::array{1, 2, 3, 4, 5};

        auto taken = flux::take(flux::ref(arr), 1'000'000);

        STATIC_CHECK(taken.usize() == arr.size());
        STATIC_CHECK(taken.data() == arr.data());
        STATIC_CHECK(check_equal(taken, arr));
    }

    // test taking zero elements
    {
        auto arr = std::array{1, 2, 3, 4, 5};

        auto taken = flux::take(flux::ref(arr), 0);

        STATIC_CHECK(taken.is_empty());
        STATIC_CHECK(taken.size() == 0);
        STATIC_CHECK(taken.distance(taken.first(), taken.last()) == 0);
        // We still want this to be true
        STATIC_CHECK(taken.data() == arr.data());
    }

    // test for_each_while impl
    {
        auto seq = flux::take(std::array{1, 2, 3, 4, 5}, 3);

        auto cur = seq.find_if(flux::pred::odd);

        STATIC_CHECK(cur == seq.first());

        cur = seq.find_if(flux::pred::even);

        STATIC_CHECK(cur == seq.next(seq.first()));

        cur = seq.find_if(flux::pred::gt(100));

        STATIC_CHECK(cur == seq.last());
        STATIC_CHECK(cur.base_cur == 3);
        STATIC_CHECK(cur.length == 0);
    }

    return true;
}
static_assert(test_take());

// Regression test for #62
// https://github.com/tcbrindle/flux/issues/62
constexpr bool issue_62()
{
    auto seq = flux::ints(0).take(5).filter(flux::pred::true_);

    STATIC_CHECK(check_equal(seq, {0, 1, 2, 3, 4}));

    return true;
}
static_assert(issue_62());

}

TEST_CASE("take")
{
    bool result = test_take();
    REQUIRE(result);

    result = issue_62();
    REQUIRE(result);

    // test taking a negative number of elements
    {
        std::list list{1, 2, 3, 4, 5};

        REQUIRE_THROWS_AS(flux::take(flux::from_range(list), -1000), flux::unrecoverable_error);

        REQUIRE_THROWS_AS(flux::from_range(list).take(-1000), flux::unrecoverable_error);
    }
}