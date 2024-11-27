
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <ranges>

#include "test_utils.hpp"

namespace {

struct ints {
    int from = 0;

    struct flux_iter_traits : flux::default_iter_traits {
        static constexpr int first(ints) { return 0; }
        static constexpr bool is_last(ints, int) { return false; }
        static constexpr int read_at(ints self, int cur){ return self.from + cur; }
        static constexpr int& inc(ints, int& cur, int o = 1) { return cur += o; }
        static constexpr int& dec(ints, int& cur) { return --cur; }
        static constexpr int distance(ints, int from, int to) { return to - from; }
    };
};

constexpr bool test_take_while()
{
    {
        auto seq = flux::take_while(ints{10}, [](int i) { return i != 25; });

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::sized_iterable<S>);

        STATIC_CHECK(check_equal(
            seq, {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24}));
    }

    {
        auto seq = flux::from(std::array{0, 1, 2, 3, 4, 5, 6, 7, 8, 9})
                        .take_while([](int i) { return i != 50; });

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::sized_iterable<S>);

        STATIC_CHECK(check_equal(seq, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }

    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        auto cur = flux::ref(arr)
                     .take_while([](int i) { return i < 5; })
                     .find(99);

        STATIC_CHECK(cur == flux::next(arr, flux::first(arr), 5));
    }

    {
        auto seq = flux::from(ints{})
                       .filter([](int i) { return i % 2 == 0; })
                       .take_while([](int i) { return i <= 10; })
                       .map([](int i) { return i * i; })
                       .drop(1);

        STATIC_CHECK(check_equal(seq, {4, 16, 36, 64, 100}));
    }

    // Test with non-sequence iterables
    {
        auto view = std::views::filter(std::array{1, 2, 3, 4, 5}, flux::pred::true_);

        auto taken = flux::take_while(std::move(view), flux::pred::lt(4));

        using T = decltype(taken);

        static_assert(flux::iterable<T>);
        static_assert(not flux::sized_iterable<T>);
        static_assert(not flux::sequence<T>);

        STATIC_CHECK(flux::contains(taken, 2));
        STATIC_CHECK(not flux::contains(taken, 4));
        STATIC_CHECK(check_equal(taken, {1, 2, 3}));
    }

    return true;
}
static_assert(test_take_while());

}

TEST_CASE("take_while")
{
    bool result = test_take_while();
    REQUIRE(result);
}