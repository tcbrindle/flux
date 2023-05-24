
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>
#include <limits>

namespace {

constexpr bool test_cycle() {

    {
        int arr[] = {1, 2, 3};

        auto seq = flux::cycle(std::ref(arr));

        using C = decltype(seq);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::infinite_sequence<C>);
        static_assert(flux::read_only_sequence<C>);
        static_assert(not flux::sized_sequence<C>); // infinite
        static_assert(not flux::bounded_sequence<C>); // infinite
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(std::same_as<flux::element_t<C>, int const&>);
        static_assert(std::same_as<flux::value_t<C>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, int const&&>);
        static_assert(std::same_as<flux::const_element_t<C>, int const&>);

        // Check the first few elements to make sure we're cycling correctly
        auto cur = seq.first();
        for (int i = 0; i < 100; i++) {
            STATIC_CHECK(seq[cur] == 1 + i % 3);
            seq.inc(cur);
        }

        // Make sure random-access works as expected
        cur = seq.first();
        auto cur2 = flux::next(seq, cur, 101);

        STATIC_CHECK(cur != cur2);
        STATIC_CHECK(cur < cur2);
        STATIC_CHECK(seq.distance(cur, cur2) == 101);

        seq.inc(cur2, -101);
        STATIC_CHECK(cur == cur2);

        // Make sure internal iteration works as expected
        int counter = 101;
        cur = flux::for_each_while(seq, [&counter](auto&&) { return counter-- > 0; });
        STATIC_CHECK(seq.distance(cur, seq.first()) == -101);
    }

    // cycle() -> take(n) is a finite sequence
    {
        int arr[] = {1, 2, 3};

        auto seq = flux::cycle(flux::ref(arr)).take(5);

        using C = decltype(seq);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::read_only_sequence<C>);
        static_assert(not flux::infinite_sequence<C>);
        static_assert(flux::sized_sequence<C>); // not infinite
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(std::same_as<flux::element_t<C>, int const&>);
        static_assert(std::same_as<flux::value_t<C>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, int const&&>);
        static_assert(std::same_as<flux::const_element_t<C>, int const&>);

        STATIC_CHECK(seq.size() == 5);

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 1, 2}));

        STATIC_CHECK(seq.sum() == 1 + 2 + 3 + 1 + 2);

        STATIC_CHECK(check_equal(flux::reverse(seq), {2, 1, 3, 2, 1}));
    }

    // zip() -> cycle() works as expected
    {
        std::array const arr1 = {1, 2, 3};
        double arr2[] = {100.0, 200.0};

        auto seq = flux::zip(arr1, flux::ref(arr2)).cycle().take(10);

        using C = decltype(seq);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::read_only_sequence<C>);
        static_assert(not flux::infinite_sequence<C>);
        static_assert(flux::sized_sequence<C>); // not infinite
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(std::same_as<flux::value_t<C>, std::pair<int, double>>);
#ifdef FLUX_HAVE_CPP23_TUPLE_COMMON_REF
        static_assert(std::same_as<flux::element_t<C>, std::pair<int const&, double const&>>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, std::pair<int const&&, double const&&>>);
        static_assert(std::same_as<flux::const_element_t<C>, std::pair<int const&, double const&>>);
#endif

        STATIC_CHECK(seq.size() == 10);

        auto firsts = flux::ref(seq).map([](auto p) { return p.first; });

        STATIC_CHECK(check_equal(firsts, {1, 2, 1, 2, 1, 2, 1, 2, 1, 2}));

        auto seconds = flux::ref(seq).map([](auto p) { return p.second; });

        STATIC_CHECK(check_equal(seconds, {100.0, 200.0, 100.0, 200.0, 100.0, 200.0, 100.0, 200.0, 100.0, 200.0}));
    }

    // cycle() on a single sequence works as expected
    {
        auto seq = flux::cycle(flux::single(3));

        using C = decltype(seq);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::infinite_sequence<C>);
        static_assert(flux::read_only_sequence<C>);
        static_assert(not flux::sized_sequence<C>); // infinite
        static_assert(not flux::bounded_sequence<C>); // infinite
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(std::same_as<flux::element_t<C>, int const&>);
        static_assert(std::same_as<flux::value_t<C>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, int const&&>);
        static_assert(std::same_as<flux::const_element_t<C>, int const&>);

        auto cur = seq.first();
        for (int i = 0; i < 10; i++) {
            STATIC_CHECK(seq[cur] == 3);
            seq.inc(cur);
        }
    }

    // cycle() on an empty sequence sort-of works
    // (This is a weird case)
    {
        auto seq = flux::cycle(flux::empty<int>);

        using C = decltype(seq);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(flux::infinite_sequence<C>);
        static_assert(flux::read_only_sequence<C>);
        static_assert(not flux::sized_sequence<C>); // infinite
        static_assert(not flux::bounded_sequence<C>); // infinite
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(std::same_as<flux::element_t<C>, int const&>);
        static_assert(std::same_as<flux::value_t<C>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, int const&&>);
        static_assert(std::same_as<flux::const_element_t<C>, int const&>);

        auto cur = seq.first();
        seq.inc(cur, 10); // compiles
    }

    // Check that cycling "infinitely" is not UB
    {
        auto seq = flux::cycle(std::array{1, 2, 3});

        auto cur = seq.first();
        cur.n = std::numeric_limits<std::size_t>::max();

        STATIC_CHECK(seq[cur] == 1);

        seq.inc(cur, 3);

        STATIC_CHECK(seq[cur] == 1);
        STATIC_CHECK(cur.n == 0);

        // Go a long way back from the start
        cur = seq.first();
        seq.inc(cur, std::numeric_limits<flux::distance_t>::lowest());
        (void) seq[cur];
    }

    // Check that for_each_while loops and terminates properly
    {
        auto seq = flux::cycle(std::array{1, 2, 3});

        int sum = 0;
        int counter = 0;

        auto cur = seq.for_each_while([&](auto&& i) {
            static_assert(std::same_as<decltype(i), int const&>);
            sum += i;
            ++counter;
            return sum < 10;
        });

        STATIC_CHECK(sum == 1 + 2 + 3 + 1 + 2 + 3);
        STATIC_CHECK(counter == 6);
        STATIC_CHECK(cur.base_cur == 2);
        STATIC_CHECK(cur.n == 1);
        STATIC_CHECK(seq[cur] == 3);
    }

    return true;
}
static_assert(test_cycle());

constexpr bool test_bounded_cycle()
{
    {
        std::array arr{1, 2, 3};

        auto seq = flux::cycle(arr, 3);

        using C = decltype(seq);

        static_assert(flux::sequence<C>);
        static_assert(flux::multipass_sequence<C>);
        static_assert(not flux::infinite_sequence<C>);
        static_assert(flux::read_only_sequence<C>);
        static_assert(flux::sized_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(flux::bidirectional_sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(std::same_as<flux::element_t<C>, int const&>);
        static_assert(std::same_as<flux::value_t<C>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, int const&&>);
        static_assert(std::same_as<flux::const_element_t<C>, int const&>);

        STATIC_CHECK(seq.size() == 9);

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 1, 2, 3, 1, 2, 3}));

        STATIC_CHECK(seq.sum() == 1 + 2 + 3 + 1 + 2 + 3 + 1 + 2 + 3);

        STATIC_CHECK(seq.front().value() == 1);
        STATIC_CHECK(seq.back().value() == 3);

        auto rev = flux::reverse(seq);

        STATIC_CHECK(check_equal(rev, {3, 2, 1, 3, 2, 1, 3, 2, 1}));
    }

    // cycle(n) on a single sequence is equivalent to cycle().take(n)
    {
        auto cycle = flux::single(10).cycle(3);
        auto take = flux::single(10).cycle().take(3);

        STATIC_CHECK(check_equal(cycle, take));
    }

    // cycle(n) on an empty sequence works
    {
        auto seq = flux::cycle(flux::empty<int>, 10);

        STATIC_CHECK(seq.size() == 0);
        STATIC_CHECK(seq.is_empty());
        STATIC_CHECK(seq.is_last(seq.first()));
        STATIC_CHECK(seq.first() == seq.last());
        STATIC_CHECK(std::is_eq(seq.first() <=> seq.last()));
        STATIC_CHECK(seq.distance(seq.first(), seq.last()) == 0);
        STATIC_CHECK(not seq.front().has_value());
        STATIC_CHECK(not seq.back().has_value());
        STATIC_CHECK(seq.sum() == 0);
    }

    // cycle(0) works as expected
    {
        auto seq = flux::cycle(std::array{1, 2, 3}, 0);

        STATIC_CHECK(seq.size() == 0);
        STATIC_CHECK(seq.is_empty());
        STATIC_CHECK(seq.is_last(seq.first()));
        STATIC_CHECK(seq.first() == seq.last());
        STATIC_CHECK(std::is_eq(seq.first() <=> seq.last()));
        STATIC_CHECK(seq.distance(seq.first(), seq.last()) == 0);
        STATIC_CHECK(not seq.front().has_value());
        STATIC_CHECK(not seq.back().has_value());
        STATIC_CHECK(seq.sum() == 0);
    }

    // test with non-bounded, non-sized sequence
    {
        auto seq = flux::take_while(std::array{1, 2, 3}, flux::pred::leq(2)).cycle(3);

        using C = decltype(seq);

        static_assert(flux::multipass_sequence<C>);
        static_assert(not flux::bidirectional_sequence<C>); // take_while is not bounded
        static_assert(not flux::infinite_sequence<C>);
        static_assert(not flux::sized_sequence<C>);
        static_assert(flux::bounded_sequence<C>); // because we can form last()

        STATIC_CHECK(seq.is_last(seq.last()));
        STATIC_CHECK(not seq.is_empty());
        STATIC_CHECK(check_equal(seq, {1, 2, 1, 2, 1, 2}));
        STATIC_CHECK(seq.sum() == 1 + 2 + 1 + 2 + 1 + 2);
        STATIC_CHECK(seq.find(2).base_cur == 1);
    }

    return true;
}
static_assert(test_bounded_cycle());

}

TEST_CASE("cycle")
{
    bool result = test_cycle();
    REQUIRE(result);

    result = test_bounded_cycle();
    REQUIRE(result);

    SECTION("negative argument to bounded cycle() is caught")
    {
        REQUIRE_THROWS_AS(flux::cycle(std::array{1, 2, 3}, -100),
                          flux::unrecoverable_error);
    }

    SECTION("over-large sizes are caught")
    {
        constexpr auto max_dist = std::numeric_limits<flux::distance_t>::max();

        auto seq = flux::ints(0, max_dist).cycle(max_dist);

        REQUIRE_THROWS_AS(seq.size(), flux::unrecoverable_error);

        REQUIRE_THROWS_AS(seq.distance(seq.first(), seq.last()), flux::unrecoverable_error);
    }
}