
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

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

    return true;
}
static_assert(test_cycle());

}

TEST_CASE("cycle")
{
    bool result = test_cycle();
    REQUIRE(result);
}